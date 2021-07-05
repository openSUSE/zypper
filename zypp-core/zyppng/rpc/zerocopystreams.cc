/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\----------------------------------------------------------------------/
*
* Some versions of protobuf lite do not export the simple zero copy streams,
* so for those we copied them from upstream.
*/

#include "zerocopystreams.h"

#ifdef PROTOBUFLITE_HAS_NO_ZEROCOPYSTREAM

// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <algorithm>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/logging.h>
#include <google/protobuf/stubs/stl_util.h>

namespace zyppng {

namespace {

// EINTR sucks.
int close_no_eintr(int fd) {
  int result;
  do {
    result = close(fd);
  } while (result < 0 && errno == EINTR);
  return result;
}

}  // namespace


// ===================================================================

FileInputStream::FileInputStream(int file_descriptor, int block_size)
  : copying_input_(file_descriptor),
    impl_(&copying_input_, block_size) {
}

bool FileInputStream::Close() {
  return copying_input_.Close();
}

bool FileInputStream::Next(const void** data, int* size) {
  return impl_.Next(data, size);
}

void FileInputStream::BackUp(int count) {
  impl_.BackUp(count);
}

bool FileInputStream::Skip(int count) {
  return impl_.Skip(count);
}

google::protobuf::int64 FileInputStream::ByteCount() const {
  return impl_.ByteCount();
}

FileInputStream::CopyingFileInputStream::CopyingFileInputStream(
    int file_descriptor)
  : file_(file_descriptor),
    close_on_delete_(false),
    is_closed_(false),
    errno_(0),
    previous_seek_failed_(false) {
}

FileInputStream::CopyingFileInputStream::~CopyingFileInputStream() {
  if (close_on_delete_) {
    if (!Close()) {
      GOOGLE_LOG(ERROR) << "close() failed: " << strerror(errno_);
    }
  }
}

bool FileInputStream::CopyingFileInputStream::Close() {
  GOOGLE_CHECK(!is_closed_);

  is_closed_ = true;
  if (close_no_eintr(file_) != 0) {
    // The docs on close() do not specify whether a file descriptor is still
    // open after close() fails with EIO.  However, the glibc source code
    // seems to indicate that it is not.
    errno_ = errno;
    return false;
  }

  return true;
}

int FileInputStream::CopyingFileInputStream::Read(void* buffer, int size) {
  GOOGLE_CHECK(!is_closed_);

  int result;
  do {
    result = read(file_, buffer, size);
  } while (result < 0 && errno == EINTR);

  if (result < 0) {
    // Read error (not EOF).
    errno_ = errno;
  }

  return result;
}

int FileInputStream::CopyingFileInputStream::Skip(int count) {
  GOOGLE_CHECK(!is_closed_);

  if (!previous_seek_failed_ &&
      lseek(file_, count, SEEK_CUR) != (off_t)-1) {
    // Seek succeeded.
    return count;
  } else {
    // Failed to seek.

    // Note to self:  Don't seek again.  This file descriptor doesn't
    // support it.
    previous_seek_failed_ = true;

    // Use the default implementation.
    return google::protobuf::io::CopyingInputStream::Skip(count);
  }
}

// ===================================================================

FileOutputStream::FileOutputStream(int file_descriptor, int block_size)
  : copying_output_(file_descriptor),
    impl_(&copying_output_, block_size) {
}

FileOutputStream::~FileOutputStream() {
  impl_.Flush();
}

bool FileOutputStream::Close() {
  bool flush_succeeded = impl_.Flush();
  return copying_output_.Close() && flush_succeeded;
}

bool FileOutputStream::Flush() {
  return impl_.Flush();
}

bool FileOutputStream::Next(void** data, int* size) {
  return impl_.Next(data, size);
}

void FileOutputStream::BackUp(int count) {
  impl_.BackUp(count);
}

google::protobuf::int64 FileOutputStream::ByteCount() const {
  return impl_.ByteCount();
}

FileOutputStream::CopyingFileOutputStream::CopyingFileOutputStream(
    int file_descriptor)
  : file_(file_descriptor),
    close_on_delete_(false),
    is_closed_(false),
    errno_(0) {
}

FileOutputStream::CopyingFileOutputStream::~CopyingFileOutputStream() {
  if (close_on_delete_) {
    if (!Close()) {
      GOOGLE_LOG(ERROR) << "close() failed: " << strerror(errno_);
    }
  }
}

bool FileOutputStream::CopyingFileOutputStream::Close() {
  GOOGLE_CHECK(!is_closed_);

  is_closed_ = true;
  if (close_no_eintr(file_) != 0) {
    // The docs on close() do not specify whether a file descriptor is still
    // open after close() fails with EIO.  However, the glibc source code
    // seems to indicate that it is not.
    errno_ = errno;
    return false;
  }

  return true;
}

bool FileOutputStream::CopyingFileOutputStream::Write(
    const void* buffer, int size) {
  GOOGLE_CHECK(!is_closed_);
  int total_written = 0;

  const google::protobuf::uint8* buffer_base = reinterpret_cast<const google::protobuf::uint8*>(buffer);

  while (total_written < size) {
    int bytes;
    do {
      bytes = write(file_, buffer_base + total_written, size - total_written);
    } while (bytes < 0 && errno == EINTR);

    if (bytes <= 0) {
      // Write error.

      // FIXME(kenton):  According to the man page, if write() returns zero,
      //   there was no error; write() simply did not write anything.  It's
      //   unclear under what circumstances this might happen, but presumably
      //   errno won't be set in this case.  I am confused as to how such an
      //   event should be handled.  For now I'm treating it as an error, since
      //   retrying seems like it could lead to an infinite loop.  I suspect
      //   this never actually happens anyway.

      if (bytes < 0) {
        errno_ = errno;
      }
      return false;
    }
    total_written += bytes;
  }

  return true;
}

// ===================================================================

} // namespace

#endif
