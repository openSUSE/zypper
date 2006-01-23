/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Buffer.cc 
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <sys/mman.h>

#include <cstdio>
#include <fcntl.h>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <ctype.h>
#include <string>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "Buffer.h"

#include "zlib.h"
#ifdef HAVE_BZ2
/* Older bzlib didn't icnlude stdio.h */
#  include <bzlib.h>
#endif

#include "zypp/base/Logger.h"


using namespace std;


//---------------------------------------------------------------------------
// compress/uncompress stuff

/*
 * Magic gunzipping goodness
 */

/*
 * Count number of bytes to skip at start of buf
 */
static int gz_magic[2] = {0x1f, 0x8b};
/* gzip flag byte */
#define GZ_ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define GZ_HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define GZ_EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define GZ_ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define GZ_COMMENT      0x10 /* bit 4 set: file comment present */
#define GZ_RESERVED     0xE0 /* bits 5..7: reserved */

static int
count_gzip_header (const unsigned char *buf, unsigned int input_length)
{
    int method, flags;
    const unsigned char *s = buf;
    unsigned int left_len = input_length;

    if (left_len < 4) return -1;
    if (*s++ != gz_magic[0] || *s++ != gz_magic[1]) {
	return -2;
    }

    method = *s++;
    flags = *s++;
    left_len -= 4;

    if (method != Z_DEFLATED || (flags & GZ_RESERVED) != 0) {
	/* If it's not deflated, or the reserved isn't 0 */
	return -3;
    }

    /* Skip time, xflags, OS code */
    if (left_len < 6) return -4;
    s += 6;
    left_len -= 6;

    if (flags & GZ_EXTRA_FIELD) {
	unsigned int len;
	if (left_len < 2) return -5;
	len = (unsigned int)(*s++);
	len += ((unsigned int)(*s++)) << 8;
	if (left_len < len) return -6;
	s += len;
	left_len -= len;
    }

    /* Skip filename */
    if (flags & GZ_ORIG_NAME) {
	while (--left_len != 0 && *s++ != '\0') ;
	if (left_len == 0) return -7;
    }
    /* Skip comment */
    if (flags & GZ_COMMENT) {
	while (--left_len != 0 && *s++ != '\0') ;
	if (left_len == 0) return -7;
    }
    /* Skip CRC */
    if (flags & GZ_HEAD_CRC) {
	if (left_len < 2) return -7;
	s += 2;
	left_len -= 2;
    }

    return input_length - left_len;
}


int
gunzip_memory (const unsigned char *input_buffer, unsigned int input_length, ByteArray **out_ba)
{
    z_stream zs;
    char *outbuf = NULL;
    ByteArray *ba = NULL;
    int zret;

    int gzip_hdr;

    if (input_buffer == NULL) return -1;
    if (input_length == 0) return -2;
    if (out_ba == NULL) return -3;

    ba = (ByteArray *)malloc (sizeof (ByteArray));
    ba->data = NULL;
    ba->len = 0;

    gzip_hdr = count_gzip_header (input_buffer, input_length);
    if (gzip_hdr < 0)
	return -1;

    zs.next_in = (unsigned char *) input_buffer + gzip_hdr;
    zs.avail_in = input_length - gzip_hdr;
    zs.zalloc = NULL;
    zs.zfree = NULL;
    zs.opaque = NULL;

#define OUTBUFSIZE 10000
    outbuf = (char *)malloc (OUTBUFSIZE);
    zs.next_out = (Bytef *)outbuf;
    zs.avail_out = OUTBUFSIZE;

    /* Negative inflateinit is magic to tell zlib that there is no
     * zlib header */
    inflateInit2 (&zs, -MAX_WBITS);

    while (1) {
	zret = inflate (&zs, Z_SYNC_FLUSH);
	if (zret != Z_OK && zret != Z_STREAM_END)
	    break;

	ba->data = (byte *)realloc (ba->data, ba->len + (OUTBUFSIZE - zs.avail_out));
	memcpy (ba->data + ba->len, outbuf, OUTBUFSIZE - zs.avail_out);
	ba->len += (OUTBUFSIZE - zs.avail_out);

	zs.next_out = (Bytef *)outbuf;
	zs.avail_out = OUTBUFSIZE;

	if (zret == Z_STREAM_END)
	    break;
    }

    inflateEnd (&zs);
    free ((void *)outbuf);

    if (zret != Z_STREAM_END) {
	ERR << "libz inflate failed! (" << zret << ")" << endl;
	free (ba->data);
	free (ba);
	ba = NULL;
    } else {
	zret = 0;
    }

    *out_ba = ba;
    return zret;
}


int
gzip_memory (const char *input_buffer, unsigned int input_length, ByteArray **out_ba)
{
    z_stream zs;
    char *outbuf = NULL;
    ByteArray *ba = NULL;
    int zret;

    if (input_buffer == NULL) return -1;
    if (input_length == 0) return -2;
    if (out_ba == NULL) return -3;

    ba = (ByteArray *)malloc (sizeof (ByteArray));
    ba->data = NULL;
    ba->len = 0;

    zs.next_in = (unsigned char *) input_buffer;
    zs.avail_in = input_length;
    zs.zalloc = NULL;
    zs.zfree = NULL;
    zs.opaque = NULL;

    outbuf = (char *)malloc (OUTBUFSIZE);
    zs.next_out = (Bytef *)outbuf;
    zs.avail_out = OUTBUFSIZE;

    deflateInit (&zs, Z_DEFAULT_COMPRESSION);

    while (1) {
	if (zs.avail_in)
	    zret = deflate (&zs, Z_SYNC_FLUSH);
	else
	    zret = deflate (&zs, Z_FINISH);
	    
	if (zret != Z_OK && zret != Z_STREAM_END)
	    break;

	ba->data = (byte *)realloc (ba->data, ba->len + (OUTBUFSIZE - zs.avail_out));
	memcpy (ba->data + ba->len, outbuf, OUTBUFSIZE - zs.avail_out);
	ba->len += (OUTBUFSIZE - zs.avail_out);

	zs.next_out = (Bytef *)outbuf;
	zs.avail_out = OUTBUFSIZE;

	if (zret == Z_STREAM_END)
	    break;
    }

    deflateEnd (&zs);
    free ((void *)outbuf);

    if (zret != Z_STREAM_END) {
	ERR << "libz deflate failed! (" << zret << ")" << endl;
	free (ba->data);
	free (ba);
	ba = NULL;
    } else {
	zret = 0;
    }

    *out_ba = ba;
    return zret;
} /* gzip_memory */


bool
memory_looks_gzipped (const unsigned char *buffer)
{
    if (buffer == NULL)
	return false;

    /* This is from RFC 1952 */

    return buffer[0] == gz_magic[0]  /* ID1 */
	&& buffer[1] == gz_magic[1]; /* ID2 */
}

/* ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** */

static char bz2_magic[3] = { 'B', 'Z', 'h' };

int
bunzip2_memory (const unsigned char *input_buffer, unsigned int input_length, ByteArray **out_ba)
{
#ifndef HAVE_BZ2

    ERR << "bz2 support not compiled in" << endl;
    *out_ba = NULL;

    return -1;

#else

    bz_stream bzs;
    ByteArray *ba;
    char *outbuf;
    int bzret;

    if (input_buffer == NULL) return -1;
    if (input_length == 0) return -2;
    if (out_ba == NULL) return -3;

    ba = (ByteArray *)malloc (sizeof (ByteArray));
    ba->data = NULL;
    ba->len = 0;

    bzs.next_in = (unsigned char *) input_buffer;
    bzs.avail_in = input_length;
    bzs.bzalloc = NULL;
    bzs.bzfree = NULL;
    bzs.opaque = NULL;

    outbuf = (char *)malloc (OUTBUFSIZE);
    bzs.next_out = (Bytef *)outbuf;
    bzs.avail_out = OUTBUFSIZE;

    BZ2_bzDecompressInit (&bzs, 1, 0);

    while (1) {
	bzret = BZ2_bzDecompress (&bzs);
	if (bzret != BZ_OK && bzret != BZ_STREAM_END)
	    break;

	ba->data = (byte *)realloc (ba->data, ba->len + (OUTBUFSIZE - zs.avail_out));
	memcpy (ba->data + ba->len, outbuf, OUTBUFSIZE - zs.avail_out);
	ba->len += (OUTBUFSIZE - zs.avail_out);

	bzs.next_out = (Bytef *)outbuf;
	bzs.avail_out = OUTBUFSIZE;

	if (bzret == BZ_STREAM_END)
	    break;

	if (bzs.avail_in == 0) {
	    /* The data is incomplete */
	    bzret = -1;
	    break;
	}
    }

    BZ2_bzDecompressEnd (&bzs);
    free ((void *)outbuf);

    if (bzret != BZ_STREAM_END) {
	ERR << "libbzip2 decompress failed (" <<  bzret << ")" << endl;
	free (ba->data);
	free (ba);
	ba = NULL;
    } else {
	bzret = 0;
    }

    *out_ba = ba;
    return bzret;
#endif
}


int
bzip2_memory (const char *input_buffer, unsigned int input_length, ByteArray **out_ba)
{
#ifndef HAVE_BZ2

    ERR << "bz2 support not compiled in" << endl;
    *out_ba = NULL;

    return -1;

#else

    bz_stream bzs;
    ByteArray *ba;
    char *outbuf;
    int bzret;

    if (input_buffer == NULL) return -1;
    if (input_length == 0) return -2;
    if (out_ba == NULL) return -3;

    ba = (ByteArray *)malloc (sizeof (ByteArray));
    ba->data = NULL;
    ba->len = 0;

    bzs.next_in = (unsigned char *) input_buffer;
    bzs.avail_in = input_length;
    bzs.bzalloc = NULL;
    bzs.bzfree = NULL;
    bzs.opaque = NULL;

    outbuf = (char *)malloc (OUTBUFSIZE);
    bzs.next_out = (Bytef *)outbuf;
    bzs.avail_out = OUTBUFSIZE;

    BZ2_bzCompressInit (&bzs, 5, 1, 0);

    while (1) {
	if (bzs.avail_in)
	    bzret = BZ2_bzCompress (&bzs, BZ_RUN);
	else
	    bzret = BZ2_bzCompress (&bzs, BZ_FINISH);
	    
	if (bzret != BZ_OK && bzret != BZ_STREAM_END)
	    break;

	ba->data = (byte *)realloc (ba->data, ba->len + (OUTBUFSIZE - zs.avail_out));
	memcpy (ba->data + ba->len, outbuf, OUTBUFSIZE - zs.avail_out);
	ba->len += (OUTBUFSIZE - zs.avail_out);

	bzs.next_out = (Bytef *)outbuf;
	bzs.avail_out = OUTBUFSIZE;

	if (bzret == BZ_STREAM_END)
	    break;
    }

    BZ2_bzCompressEnd (&bzs);
    free ((void *)outbuf);

    if (bzret != BZ_STREAM_END) {
	ERR << "bz2 compress failed! (" << bzret << ")" << endl;
	free (ba->data);
	free (ba);
	ba = NULL;
    } else {
	bzret = 0;
    }

    *out_ba = ba;
    return bzret;
#endif
}


bool
memory_looks_bzip2ed (const unsigned char *buffer)
{
    if (buffer == NULL)
	return false;

    return buffer[0] == bz2_magic[0]
	&& buffer[1] == bz2_magic[1]
	&& buffer[2] == bz2_magic[2];
}

/* ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** */

int
uncompress_memory (const unsigned char *input_buffer, unsigned int input_length, ByteArray **out_ba)
{
    if (input_length > 2 && memory_looks_bzip2ed (input_buffer))
	return bunzip2_memory (input_buffer, input_length, out_ba);
    else if (input_length > 3 && memory_looks_gzipped (input_buffer))
	return gunzip_memory (input_buffer, input_length, out_ba);
    else
	return -1;
}

bool
memory_looks_compressed (const unsigned char *buffer, size_t size)
{
#ifdef HAVE_BZ2
    if (size > 2 && memory_looks_bzip2ed (buffer))
	return true;
#endif

    if (size > 4 && memory_looks_gzipped (buffer))
	return true;

    return false;
}

//---------------------------------------------------------------------------
// I/O stuff

/* 
 * This just allows reading from the buffer for now.  It could be extended to
 * do writing if necessary.
 */

Buffer *
bufferMapFile (const string & filename)
{
    struct stat s;
    int fd;
    unsigned char *data;
    Buffer *buf = NULL;

    if (filename.empty())
	return NULL;

    if (stat(filename.c_str(), &s) < 0)
	return NULL;

    fd = open(filename.c_str(), O_RDONLY);

    if (fd < 0)
	return NULL;

    data = (unsigned char *)mmap(NULL, s.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);

    close (fd);

    if (data == MAP_FAILED)
	return NULL;

    /* Transparently uncompress */
    if (memory_looks_compressed (data, s.st_size)) {
	ByteArray *byte_array = NULL;

	if (uncompress_memory (data, s.st_size, &byte_array)) {
	    WAR << "Uncompression of '" << filename << "' failed" << endl;
	} else {
	    buf = (Buffer *)malloc(sizeof (Buffer));
	    buf->data       = byte_array->data;
	    buf->size       = byte_array->len;
	    buf->is_mmapped = false;
	}

	munmap (data, s.st_size);

	if (byte_array) {
	    free (byte_array);
	}

    } else {
	buf = (Buffer *)malloc(sizeof (Buffer));
	buf->data       = (byte *)data;
	buf->size       = s.st_size;
	buf->is_mmapped = true;
    }

    return buf;
} /* bufferMapFile */


void
bufferUnmapFile (Buffer *buf)
{
    if (buf == NULL) return;

    if (buf->is_mmapped)
	munmap (buf->data, buf->size);
    else
	free (buf->data);

    free (buf);
}

//---------------------------------------------------------------------------
// XML stuff

xmlDoc *
parseXmlFromBuffer (const char *input_buffer, size_t input_length)
{
    xmlDoc *doc = NULL;

    if (input_buffer == NULL) return NULL;

    if (input_length > 3 && memory_looks_gzipped ((const unsigned char *)input_buffer)) { 
        ByteArray *buf;

        if (uncompress_memory ((const unsigned char *)input_buffer, input_length, &buf)) {
            return NULL;
        }
        doc = xmlParseMemory ((const char *)(buf->data), buf->len);
	free (buf->data);
        free (buf);
    } else {
        doc = xmlParseMemory (input_buffer, input_length);
    }

    return doc;
}


xmlDoc *
parseXmlFromFile (const string & filename)
{
    Buffer *buf;
    xmlDoc *doc = NULL;

    if (filename.empty()) return NULL;

    buf = bufferMapFile (filename);
    if (buf) {
        doc = xmlParseMemory ((const char *)(buf->data), buf->size);
        bufferUnmapFile (buf);
    }

    return doc;
}
