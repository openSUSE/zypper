/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* DbSourceImpl.h
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

#ifndef ZYPP_ZMD_BACKEND_DBSOURCEIMPL_H
#define ZYPP_ZMD_BACKEND_DBSOURCEIMPL_H

#include <iosfwd>
#include <string>

#include "zypp/source/SourceImpl.h"

class DbReader;
//#include "DbReader.h"

#include "zypp/Package.h"
#include "zypp/Message.h"
#include "zypp/Script.h"
#include "zypp/Patch.h"
#include "zypp/Product.h"
#include "zypp/Selection.h"
#include "zypp/Pattern.h"

namespace zypp {
        
///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : DbSourceImpl

class DbSourceImpl : public zypp::source::SourceImpl {

  public:

    /** Default ctor */
    DbSourceImpl(media::MediaAccess::Ptr & media_r, const Pathname & path_r = "/", const std::string & alias_r = "");

    virtual const bool valid() const
    { return true; }

    Package::Ptr createPackage (const DbReader & data);
    Message::Ptr createMessage (const DbReader & data);
    Script::Ptr  createScript (const DbReader & data);
    Patch::Ptr   createPatch (const DbReader & data);
    Pattern::Ptr createPattern (const DbReader & data);
    Product::Ptr createProduct (const DbReader & data);

    Dependencies createDependencies (const DbReader & data);

  private:
    Source_Ref _source;
    const Pathname _pathname;
    void createResolvables(Source_Ref source_r);
};


} // namespace zypp

#endif // ZYPP_ZMD_BACKEND_DBSOURCEIMPL_H
