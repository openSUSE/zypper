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
#include "HelixScriptImpl.h"
#include "HelixMessageImpl.h"
#include "HelixPatchImpl.h"
#include "HelixPatternImpl.h"
#include "HelixProductImpl.h"

#include "zypp/source/SourceImpl.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"


using namespace std;
using namespace zypp;

//---------------------------------------------------------------------------

HelixSourceImpl::HelixSourceImpl(media::MediaId & mediaid_r, const Pathname & path_r, const std::string & alias_r)
    : SourceImpl (mediaid_r, path_r, alias_r)
    , _source (Source_Ref::noSource)
    , _pathname (path_r)
{
}


void
HelixSourceImpl::createResolvables(Source_Ref source_r)
{
    _source = source_r;
    extractHelixFile (_pathname.asString(), this);
}



//-----------------------------------------------------------------------------

Dependencies
HelixSourceImpl::createDependencies (const HelixParser & parsed)
{
    Dependencies deps;

    deps[Dep::PROVIDES] = parsed.provides;
    deps[Dep::PREREQUIRES] = parsed.prerequires;
    deps[Dep::REQUIRES] = parsed.requires;
    deps[Dep::CONFLICTS] = parsed.conflicts;
    deps[Dep::OBSOLETES] = parsed.obsoletes;
    deps[Dep::RECOMMENDS] = parsed.recommends;
    deps[Dep::SUGGESTS] = parsed.suggests;
    deps[Dep::FRESHENS] = parsed.freshens;
    deps[Dep::ENHANCES] = parsed.enhances;

    return deps;
}


Package::Ptr
HelixSourceImpl::createPackage (const HelixParser & parsed)
{
    try
    {
	shared_ptr<HelixPackageImpl> impl(new HelixPackageImpl(_source, parsed));

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
HelixSourceImpl::createMessage (const HelixParser & parsed)
{
    try
    {
	shared_ptr<HelixMessageImpl> impl(new HelixMessageImpl(_source, parsed));

	// Collect basic Resolvable data
	NVRAD dataCollect( parsed.name,
			Edition( parsed.version, parsed.release, parsed.epoch ),
			Arch( parsed.arch ),
			createDependencies (parsed));
	Message::Ptr message = detail::makeResolvableFromImpl(dataCollect, impl);
	return message;
    }
    catch (const Exception & excpt_r)
    {
	ERR << excpt_r << endl;
	throw "Cannot create message object";
    }
    return NULL;
}


Script::Ptr
HelixSourceImpl::createScript (const HelixParser & parsed)
{
    try
    {
	shared_ptr<HelixScriptImpl> impl(new HelixScriptImpl(_source, parsed));

	// Collect basic Resolvable data
	NVRAD dataCollect( parsed.name,
			Edition( parsed.version, parsed.release, parsed.epoch ),
			Arch( parsed.arch ),
			createDependencies (parsed));
	Script::Ptr script = detail::makeResolvableFromImpl(dataCollect, impl);
	return script;
    }
    catch (const Exception & excpt_r)
    {
	ERR << excpt_r << endl;
	throw "Cannot create script object";
    }
    return NULL;
}


Patch::Ptr
HelixSourceImpl::createPatch (const HelixParser & parsed)
{
    try
    {
	shared_ptr<HelixPatchImpl> impl(new HelixPatchImpl(_source, parsed));

	// Collect basic Resolvable data
	NVRAD dataCollect( parsed.name,
			Edition( parsed.version, parsed.release, parsed.epoch ),
			Arch( parsed.arch ),
			createDependencies (parsed));
	Patch::Ptr patch = detail::makeResolvableFromImpl(dataCollect, impl);
	return patch;
    }
    catch (const Exception & excpt_r)
    {
	ERR << excpt_r << endl;
	throw "Cannot create patch object";
    }
    return NULL;
}


Pattern::Ptr
HelixSourceImpl::createPattern (const HelixParser & parsed)
{
    try
    {
	shared_ptr<HelixPatternImpl> impl(new HelixPatternImpl(_source, parsed));

	// Collect basic Resolvable data
	NVRAD dataCollect( parsed.name,
			Edition( parsed.version, parsed.release, parsed.epoch ),
			Arch( parsed.arch ),
			createDependencies (parsed));
	Pattern::Ptr pattern = detail::makeResolvableFromImpl(dataCollect, impl);
	return pattern;
    }
    catch (const Exception & excpt_r)
    {
	ERR << excpt_r << endl;
	throw "Cannot create pattern object";
    }
    return NULL;
}


Product::Ptr
HelixSourceImpl::createProduct (const HelixParser & parsed)
{
    try
    {
	shared_ptr<HelixProductImpl> impl(new HelixProductImpl(_source, parsed));

	// Collect basic Resolvable data
	NVRAD dataCollect( parsed.name,
			Edition( parsed.version, parsed.release, parsed.epoch ),
			Arch( parsed.arch ),
			createDependencies (parsed));
	Product::Ptr product = detail::makeResolvableFromImpl(dataCollect, impl);
	return product;
    }
    catch (const Exception & excpt_r)
    {
	ERR << excpt_r << endl;
	throw "Cannot create product object";
    }
    return NULL;
}

//-----------------------------------------------------------------------------

void
HelixSourceImpl::parserCallback (const HelixParser & parsed)
{
  try {
    if (parsed.kind == ResTraits<Package>::kind) {
	Package::Ptr p = createPackage (parsed);
	_store.insert (p);
    }
    else if (parsed.kind == ResTraits<Message>::kind) {
	Message::Ptr m = createMessage (parsed);
	_store.insert (m);
    }
    else if (parsed.kind == ResTraits<Script>::kind) {
	Script::Ptr s = createScript (parsed);
	_store.insert (s);
    }
    else if (parsed.kind == ResTraits<Patch>::kind) {
	Patch::Ptr p = createPatch (parsed);
	_store.insert (p);
    }
    else if (parsed.kind == ResTraits<Pattern>::kind) {
	Pattern::Ptr p = createPattern (parsed);
	_store.insert (p);
    }
    else if (parsed.kind == ResTraits<Product>::kind) {
	Product::Ptr p = createProduct (parsed);
	_store.insert (p);
    }
    else {
	ERR << "Unsupported kind " << parsed.kind << endl;
    }
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT (excpt_r);
  }
}

//-----------------------------------------------------------------------------

