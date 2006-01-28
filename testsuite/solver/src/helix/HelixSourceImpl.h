/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* HelixSourceImpl.h
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

#ifndef ZYPP_SOLVER_TEMPORARY_HELIXSOURCEIMPL_H
#define ZYPP_SOLVER_TEMPORARY_HELIXSOURCEIMPL_H

#include <iosfwd>
#include <string>

#include "zypp/source/SourceImpl.h"

#include "HelixParser.h"

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
//	CLASS NAME : HelixSourceImpl

class HelixSourceImpl : public zypp::source::SourceImpl {

  public:

    /** Ctor, FIXME it is here only because of target storage */
    HelixSourceImpl()
      : _source (Source::nullimpl())
    {}
    /** Default ctor */
    HelixSourceImpl(media::MediaAccess::Ptr & media_r, const Pathname & path_r = "/", const std::string & alias_r = "");

    Package::Ptr createPackage (const HelixParser & data);
    Message::Ptr createMessage (const HelixParser & data);
    Script::Ptr  createScript (const HelixParser & data);
    Patch::Ptr   createPatch (const HelixParser & data);
    Pattern::Ptr createPattern (const HelixParser & data);
    Product::Ptr createProduct (const HelixParser & data);

    Dependencies createDependencies (const HelixParser & data);

    void parserCallback (const HelixParser & data);

  private:
    Source_Ref _source;
    const Pathname _pathname;
    void createResolvables(Source_Ref source_r);

};


} // namespace zypp

#endif // ZYPP_SOLVER_TEMPORARY_HELIXSOURCEIMPL_H
