/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
* Some versions of protobuf lite do not export the simple zero copy streams,
* so for those we copied them from upstream.
*/
#ifndef ZYPP_NG_RPC_ZEROCOPYSTREAMS_H_INCLUDED
#define ZYPP_NG_RPC_ZEROCOPYSTREAMS_H_INCLUDED

#ifndef PROTOBUFLITE_HAS_NO_ZEROCOPYSTREAM

// just use the one we get from libprotobuf
#include <google/protobuf/io/zero_copy_stream_impl.h>

// pull them into our namespace
namespace zyppng {
    using FileInputStream  = google::protobuf::io::FileInputStream;
    using FileOutputStream = google::protobuf::io::FileOutputStream;
}

#else

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
//
// This file contains common implementations of the interfaces defined in
// zero_copy_stream.h which are only included in the full (non-lite)
// protobuf library.  These implementations include Unix file descriptors
// and C++ iostreams.  See also:  zero_copy_stream_impl_lite.h

#include <string>
#include <iosfwd>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/stubs/common.h>

// pull into zyppng namespace, even though protobuf-lite does not export the symbols of those
// classes the headers still define them. So make sure we do not clash.
namespace zyppng {

// A ZeroCopyInputStream which reads from a file descriptor.
//
// FileInputStream is preferred over using an ifstream with IstreamInputStream.
// The latter will introduce an extra layer of buffering, harming performance.
// Also, it's conceivable that FileInputStream could someday be enhanced
// to use zero-copy file descriptors on OSs which support them.
class FileInputStream : public google::protobuf::io::ZeroCopyInputStream {
 public:
  // Creates a stream that reads from the given Unix file descriptor.
  // If a block_size is given, it specifies the number of bytes that
  // should be read and returned with each call to Next().  Otherwise,
  // a reasonable default is used.
  explicit FileInputStream(int file_descriptor, int block_size = -1);

  // Flushes any buffers and closes the underlying file.  Returns false if
  // an error occurs during the process; use GetErrno() to examine the error.
  // Even if an error occurs, the file descriptor is closed when this returns.
  bool Close();

  // By default, the file descriptor is not closed when the stream is
  // destroyed.  Call SetCloseOnDelete(true) to change that.  WARNING:
  // This leaves no way for the caller to detect if close() fails.  If
  // detecting close() errors is important to you, you should arrange
  // to close the descriptor yourself.
  void SetCloseOnDelete(bool value) { copying_input_.SetCloseOnDelete(value); }

  // If an I/O error has occurred on this file descriptor, this is the
  // errno from that error.  Otherwise, this is zero.  Once an error
  // occurs, the stream is broken and all subsequent operations will
  // fail.
  int GetErrno() { return copying_input_.GetErrno(); }

  // implements ZeroCopyInputStream ----------------------------------
  bool Next(const void** data, int* size);
  void BackUp(int count);
  bool Skip(int count);
  google::protobuf::int64 ByteCount() const;

 private:
  class CopyingFileInputStream : public google::protobuf::io::CopyingInputStream {
   public:
    CopyingFileInputStream(int file_descriptor);
    ~CopyingFileInputStream();

    bool Close();
    void SetCloseOnDelete(bool value) { close_on_delete_ = value; }
    int GetErrno() { return errno_; }

    // implements CopyingInputStream ---------------------------------
    int Read(void* buffer, int size);
    int Skip(int count);

   private:
    // The file descriptor.
    const int file_;
    bool close_on_delete_;
    bool is_closed_;

    // The errno of the I/O error, if one has occurred.  Otherwise, zero.
    int errno_;

    // Did we try to seek once and fail?  If so, we assume this file descriptor
    // doesn't support seeking and won't try again.
    bool previous_seek_failed_;

    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(CopyingFileInputStream);
  };

  CopyingFileInputStream copying_input_;
  google::protobuf::io::CopyingInputStreamAdaptor impl_;

  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(FileInputStream);
};

// ===================================================================

// A ZeroCopyOutputStream which writes to a file descriptor.
//
// FileOutputStream is preferred over using an ofstream with
// OstreamOutputStream.  The latter will introduce an extra layer of buffering,
// harming performance.  Also, it's conceivable that FileOutputStream could
// someday be enhanced to use zero-copy file descriptors on OSs which
// support them.
class FileOutputStream : public google::protobuf::io::ZeroCopyOutputStream {
 public:
  // Creates a stream that writes to the given Unix file descriptor.
  // If a block_size is given, it specifies the size of the buffers
  // that should be returned by Next().  Otherwise, a reasonable default
  // is used.
  explicit FileOutputStream(int file_descriptor, int block_size = -1);
  ~FileOutputStream();

  // Flushes any buffers and closes the underlying file.  Returns false if
  // an error occurs during the process; use GetErrno() to examine the error.
  // Even if an error occurs, the file descriptor is closed when this returns.
  bool Close();

  // Flushes FileOutputStream's buffers but does not close the
  // underlying file. No special measures are taken to ensure that
  // underlying operating system file object is synchronized to disk.
  bool Flush();

  // By default, the file descriptor is not closed when the stream is
  // destroyed.  Call SetCloseOnDelete(true) to change that.  WARNING:
  // This leaves no way for the caller to detect if close() fails.  If
  // detecting close() errors is important to you, you should arrange
  // to close the descriptor yourself.
  void SetCloseOnDelete(bool value) { copying_output_.SetCloseOnDelete(value); }

  // If an I/O error has occurred on this file descriptor, this is the
  // errno from that error.  Otherwise, this is zero.  Once an error
  // occurs, the stream is broken and all subsequent operations will
  // fail.
  int GetErrno() { return copying_output_.GetErrno(); }

  // implements ZeroCopyOutputStream ---------------------------------
  bool Next(void** data, int* size);
  void BackUp(int count);
  google::protobuf::int64 ByteCount() const;

 private:
  class CopyingFileOutputStream : public google::protobuf::io::CopyingOutputStream {
   public:
    CopyingFileOutputStream(int file_descriptor);
    ~CopyingFileOutputStream();

    bool Close();
    void SetCloseOnDelete(bool value) { close_on_delete_ = value; }
    int GetErrno() { return errno_; }

    // implements CopyingOutputStream --------------------------------
    bool Write(const void* buffer, int size);

   private:
    // The file descriptor.
    const int file_;
    bool close_on_delete_;
    bool is_closed_;

    // The errno of the I/O error, if one has occurred.  Otherwise, zero.
    int errno_;

    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(CopyingFileOutputStream);
  };

  CopyingFileOutputStream copying_output_;
  google::protobuf::io::CopyingOutputStreamAdaptor impl_;

  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(FileOutputStream);
};

}  // namespace

#endif



#endif
