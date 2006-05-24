/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 * Buffer.h
 *
 * Copyright (C) 2003 Ximian, Inc.
 * Copyright (c) 2005 SUSE Linux Products GmbH
 *
 */

/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <sys/types.h>
#include <string>

typedef unsigned char byte;

typedef struct {
    byte *data;
    size_t len;
} ByteArray;

// An easy way to map files.  If we map a compressed file,
//   it will be magically uncompressed for us.

typedef struct {
    byte *data;
    size_t size;
    bool is_mmapped;
} Buffer;

Buffer *bufferMapFile (const std::string & filename);
void bufferUnmapFile (Buffer *buffer);


xmlDoc *parseXmlFromBuffer (const char *input_buffer, size_t input_length);
xmlDoc *parseXmlFromFile (const std::string & filename);


#endif // BUFFER_H

