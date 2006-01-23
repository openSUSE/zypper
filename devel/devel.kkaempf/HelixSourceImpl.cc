/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* HelixSourceImpl.cc
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

#include "HelixExtract.h"
#include "HelixSourceImpl.h"
#include "HelixPackageImpl.h"

#include "zypp/solver/temporary/Channel.h"
#include "zypp/solver/temporary/extract.h"
#include "zypp/base/Logger.h"


using namespace std;
using namespace zypp;

//---------------------------------------------------------------------------

HelixSourceImpl::HelixSourceImpl(media::MediaAccess::Ptr & media_r, const Pathname & path_r)
{
    load (path_r.asString());
}

Dependencies
HelixSourceImpl::createDependencies (const HelixParser & parsed)
{
    Dependencies deps;

    deps.provides = parsed.provides;
    deps.prerequires = parsed.prerequires;
    deps.requires = parsed.requires;
    deps.conflicts = parsed.conflicts;
    deps.obsoletes = parsed.obsoletes;
    deps.recommends = parsed.recommends;
    deps.suggests = parsed.suggests;
    deps.freshens = parsed.freshens;
    deps.enhances = parsed.enhances;

    return deps;
}

Package::Ptr
HelixSourceImpl::createPackage (const HelixParser & parsed)
{
    try
    {
	shared_ptr<HelixPackageImpl> impl(new HelixPackageImpl(parsed));

	// Collect basic Resolvable data
	NVRAD dataCollect( parsed.name,
			Edition( parsed.version, parsed.release, parsed.epoch ),
			Arch( parsed.arch ),
			createDependencies (parsed));
	Package::Ptr package = detail::makeResolvableFromImpl(dataCollect, impl);
	return package;
    }
    catch (const Exception & excpt_r)
    {
	ERR << excpt_r << endl;
	throw "Cannot create package object";
    }
    return NULL;
}


Message::Ptr
HelixSourceImpl::createMessage (const HelixParser & data)
{
    return NULL;
}


Script::Ptr
HelixSourceImpl::createScript (const HelixParser & data)
{
    return NULL;
}


Patch::Ptr
HelixSourceImpl::createPatch (const HelixParser & data)
{
    return NULL;
}


Pattern::Ptr
HelixSourceImpl::createPattern (const HelixParser & data)
{
    return NULL;
}


Product::Ptr
HelixSourceImpl::createProduct (const HelixParser & data)
{
    return NULL;
}

//-----------------------------------------------------------------------------

void
HelixSourceImpl::parserCallback (const HelixParser & data)
{
    if (data.kind == ResTraits<Package>::kind) {
	Package::Ptr p = createPackage (data);
	_store.insert (p);
    }
}

//-----------------------------------------------------------------------------

void
HelixSourceImpl::load (const string & filename)
{
    if (!filename.empty()) {
	solver::detail::Channel_Ptr channel = new solver::detail::Channel ("test", "test", "test", "test");
	channel->setType (solver::detail::CHANNEL_TYPE_HELIX);
//	channel->setSystem (true);

	extractHelixFile (filename, channel, this);
    }
}
