/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 * utils.h
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

#include <sys/types.h>
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>
/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////
      
      typedef unsigned char byte;
      
      char *strstrip (const char *str);
      char *maybe_merge_paths(const char *parent_path, const char *child_path);
      bool url_is_absolute (const char *url);
      
      
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
      
      Buffer *buffer_map_file (const std::string & filename);
      void buffer_unmap_file (Buffer *buffer);
      
      
      xmlDoc *parse_xml_from_buffer (const char *input_buffer, size_t input_length);
      xmlDoc *parse_xml_from_file (const std::string & filename);
      
      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

