/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 * extract.h
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

#ifndef ZYPP_SOLVER_TEMPORARY_EXTRACT_H
#define ZYPP_SOLVER_TEMPORARY_EXTRACT_H

#include "zypp/solver/detail/Pending.h"

#include "zypp/solver/temporary/Channel.h"
#include "zypp/solver/temporary/XmlNode.h"
#include "zypp/solver/temporary/ResItem.h"
#include "zypp/solver/temporary/Package.h"
#include "zypp/solver/temporary/Match.h"
#include "zypp/solver/temporary/StoreWorldPtr.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

int extract_packages_from_helix_buffer (const char data[], size_t len, Channel_Ptr channel, CResItemFn callback, void *data);
int extract_packages_from_helix_file (const std::string & filename, Channel_Ptr channel, CResItemFn callback, void *data);

int extract_packages_from_undump_buffer (const char *data, size_t len, ChannelAndSubscribedFn channel_callback, CResItemFn package_callback, MatchFn lock_callback, void *data);
int extract_packages_from_undump_file (const std::string & filename, ChannelAndSubscribedFn channel_callback, CResItemFn package_callback, MatchFn lock_callback, void *data);

int extract_packages_from_xml_node (XmlNode_constPtr node, Channel_Ptr channel, ResItemFn callback, void *data);

#if 0
int extract_packages_from_debian_buffer (const char *data, size_t len, Channel_Ptr channel, CResItemFn callback, void *data);
int extract_packages_from_debian_file (const std::string & filename, Channel_Ptr channel, CResItemFn callback, void *data);

Package_Ptr extract_yum_package (const char *data, size_t len, Packman_Ptr packman, const std::string & url);

int extract_packages_from_aptrpm_buffer (const char *data, size_t len, Packman_Ptr packman, Channel_Ptr channel, ResItemFn callback, void *data);
int extract_packages_from_aptrpm_file (const std::string & filename, Packman_Ptr packman, Channel_Ptr channel, ResItemFn callback, void *data);

int extract_packages_from_directory (const std::string & path, Channel_Ptr channel, Packman_Ptr packman, bool recursive, ResItemFn callback, void *data);
#endif

///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////
#endif /* ZYPP_SOLVER_TEMPORARY_EXTRACT_H */

