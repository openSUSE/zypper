/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

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

#ifndef __EXTRACT_H__
#define __EXTRACT_H__

#include <zypp/solver/detail/Channel.h>
#include <zypp/solver/detail/XmlNode.h>
#include <zypp/solver/detail/Channel.h>
#include <zypp/solver/detail/Pending.h>
#include <zypp/solver/detail/ResItem.h>
#include <zypp/solver/detail/Package.h>
#include <zypp/solver/detail/Match.h>
#include <zypp/solver/detail/StoreWorldPtr.h>
#include <zypp/solver/detail/PackmanPtr.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////


int extract_packages_from_helix_buffer (const char data[], size_t len, ChannelPtr channel, CResItemFn callback, void *data);
int extract_packages_from_helix_file (const std::string & filename, ChannelPtr channel, CResItemFn callback, void *data);

int extract_packages_from_xml_node (constXmlNodePtr node, ChannelPtr channel, ResItemFn callback, void *data);

int extract_packages_from_debian_buffer (const char *data, size_t len, ChannelPtr channel, CResItemFn callback, void *data);
int extract_packages_from_debian_file (const std::string & filename, ChannelPtr channel, CResItemFn callback, void *data);

PackagePtr extract_yum_package (const char *data, size_t len, PackmanPtr packman, const std::string & url);

int extract_packages_from_aptrpm_buffer (const char *data, size_t len, PackmanPtr packman, ChannelPtr channel, ResItemFn callback, void *data);
int extract_packages_from_aptrpm_file (const std::string & filename, PackmanPtr packman, ChannelPtr channel, ResItemFn callback, void *data);

int extract_packages_from_undump_buffer (const char *data, size_t len, ChannelAndSubscribedFn channel_callback, CResItemFn package_callback, MatchFn lock_callback, void *data);
int extract_packages_from_undump_file (const std::string & filename, ChannelAndSubscribedFn channel_callback, CResItemFn package_callback, MatchFn lock_callback, void *data);

int extract_packages_from_directory (const std::string & path, ChannelPtr channel, PackmanPtr packman, bool recursive, ResItemFn callback, void *data);

///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

#endif /* __EXTRACT_H__ */

