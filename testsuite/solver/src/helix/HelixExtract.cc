/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 * extract.cc
 *
 * Copyright (C) 2000-2003 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
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

#include "HelixExtract.h"
#include "HelixParser.h"
#include "Buffer.h"

#include "zypp/base/Logger.h"

using namespace std;
using namespace zypp;

namespace zypp {

int 
extractHelixBuffer (const char *buf, size_t len, HelixSourceImpl *impl)
{
//    _DBG("HelixExtract") << "extract_packages_from_helix_buffer(" << buf << "...," << (long)len << ",...)" << endl;

    if (buf == NULL || len == 0)
	return 1;

    HelixParser parser;
    parser.parseChunk (buf, len, impl);
    parser.done ();

    return 0;
}


int
extractHelixFile (const std::string & filename, HelixSourceImpl *impl)
{
    Buffer *buf;

    if (filename.empty())
	return -1;

    buf = bufferMapFile (filename);
    if (buf == NULL)
	return -1;

    extractHelixBuffer ((const char *)(buf->data), buf->size, impl);

    bufferUnmapFile (buf);

    return 0;
}

} // namespace zypp
