/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* DbSourceImpl.cc
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

#include "DbSourceImpl.h"

#include "DbPackageImpl.h"
#if 0
#include "DbScriptImpl.h"
#include "DbMessageImpl.h"
#include "DbPatchImpl.h"
#include "DbPatternImpl.h"
#include "DbProductImpl.h"
#endif

#include "zypp/source/SourceImpl.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/CapFactory.h"

using namespace std;
using namespace zypp;

//---------------------------------------------------------------------------

DbSourceImpl::DbSourceImpl()
    : _db (NULL)
    , _idmap (NULL)
{
}


void
DbSourceImpl::factoryInit()
{
    MIL << "DbSourceImpl::factoryInit()" << endl;

    try {
	media::MediaManager media_mgr;
	MIL << "Adding no media verifier" << endl;
	media::MediaAccessId _media = _media_set->getMediaAccessId(1);
	media_mgr.delVerifier(_media);
	media_mgr.addVerifier(_media, media::MediaVerifierRef(new media::NoVerifier()));
    }
    catch (const Exception & excpt_r)
    {
#warning FIXME: If media data is not set, verifier is not set. Should the media be refused instead?
	ZYPP_CAUGHT(excpt_r);
	WAR << "Verifier not found" << endl;
    }
    return;
}

void
DbSourceImpl::attachDatabase( sqlite3 *db)
{
    _db = db;
}

void
DbSourceImpl::attachIdMap (IdMap *idmap)
{
  _idmap = idmap;
}

//-----------------------------------------------------------------------------

static sqlite3_stmt *
create_dependency_handle (sqlite3 *db)
{
    const char *query;
    int rc;
    sqlite3_stmt *handle = NULL;

    query =
	//	0         1     2        3        4      5     6         7
        "SELECT dep_type, name, version, release, epoch, arch, relation, dep_target "
        "FROM dependencies "
        "WHERE resolvable_id = ?";

    rc = sqlite3_prepare (db, query, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not prepare dependency selection clause: " << sqlite3_errmsg (db) << endl;
        sqlite3_finalize (handle);
        return NULL;
    }

    return handle;
}

static sqlite3_stmt *
create_package_handle (sqlite3 *db)
{
    const char *query;
    int rc;
    sqlite3_stmt *handle = NULL;

    query =
	//      0   1     2        3        4      5
        "SELECT id, name, version, release, epoch, arch, "
	//      6               7
        "       installed_size, catalog,"
	//      8          9      10         11
        "       installed, local, rpm_group, file_size,"
	//      12       13           14
        "       summary, description, package_filename,"
	//      15
        "       install_only "
        "FROM packages "
        "WHERE catalog = ?";

    rc = sqlite3_prepare (db, query, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not prepare package_details selection clause: " << sqlite3_errmsg (db) << endl;
        sqlite3_finalize (handle);
        return NULL;
    }

    return handle;
}


void
DbSourceImpl::createResolvables(Source_Ref source_r)
{
    MIL << "DbSourceImpl::createResolvables(" << source_r.id() << ")" << endl;
    _source = source_r;
    if (_db == NULL) {
	ERR << "Must call attachDatabase() first" << endl;
	return;
    }

    _dependency_handle = create_dependency_handle (_db);
    if (_dependency_handle == NULL) return;

    createPackages();

    return;
}


void
DbSourceImpl::createPackages(void)
{
    sqlite3_stmt *handle = create_package_handle (_db);
    if (handle == NULL) return;

    sqlite3_bind_text (handle, 1, _source.id().c_str(), -1, SQLITE_STATIC);

    int rc;
    while ((rc = sqlite3_step (handle)) == SQLITE_ROW) {

	string name;

	try
	{
	    detail::ResImplTraits<DbPackageImpl>::Ptr impl( new DbPackageImpl( _source ) );

	    sqlite_int64 id = sqlite3_column_int64( handle, 0 );
	    name = (const char *) sqlite3_column_text( handle, 1 );
	    string version ((const char *) sqlite3_column_text( handle, 2 ));
	    string release ((const char *) sqlite3_column_text( handle, 3 ));
	    unsigned epoch = sqlite3_column_int( handle, 4 );
	    Arch arch( DbAccess::Rc2Arch( (RCArch)(sqlite3_column_int( handle, 5 )) ) );

	    impl->readHandle( id, handle );

	    // Collect basic Resolvable data
	    NVRAD dataCollect( name,
			Edition( version, release, epoch ),
			arch,
			createDependencies (id));

	    Package::Ptr package = detail::makeResolvableFromImpl( dataCollect, impl );
	    _store.insert( package );
	    if (_idmap != 0)
		(*_idmap)[id] = package;
	}
	catch (const Exception & excpt_r)
	{
	    ERR << "Cannot create package object '"+name+"' from catalog '"+_source.id()+"'" << endl;
	    sqlite3_reset (handle);
	    ZYPP_RETHROW (excpt_r);
	}
    }

    sqlite3_reset (handle);
    return;
}

//-----------------------------------------------------------------------------

// convert ZMD RCDependencyTarget to ZYPP Resolvable kind
static Resolvable::Kind 
target2kind( RCDependencyTarget dep_target )
{
    Resolvable::Kind kind;

    switch (dep_target)
    {
	case RC_DEP_TARGET_PACKAGE:	kind = ResTraits<Package>::kind;
	break;
	case RC_DEP_TARGET_SCRIPT:	kind = ResTraits<Package>::kind;
	break;
	case RC_DEP_TARGET_MESSAGE:	kind = ResTraits<Message>::kind;
	break;
	case RC_DEP_TARGET_PATCH:	kind = ResTraits<Patch>::kind;
	break;
	case RC_DEP_TARGET_SELECTION:	kind = ResTraits<Selection>::kind;
	break;
	case RC_DEP_TARGET_PATTERN:	kind = ResTraits<Pattern>::kind;
	break;
	case RC_DEP_TARGET_PRODUCT:	kind = ResTraits<Product>::kind;
        break;
	default:			WAR << "Unknown dep_target " << dep_target << endl;
					kind = ResTraits<Package>::kind;
	break;
    }
    return kind;
}


Dependencies
DbSourceImpl::createDependencies (sqlite_int64 resolvable_id)
{
    Dependencies deps;
    CapFactory factory;

    sqlite3_bind_int64 (_dependency_handle, 1, resolvable_id);

    RCDependencyType dep_type;
    string name, version, release;
    unsigned epoch;
    Arch arch;
    Rel rel;
    Capability cap;
    Resolvable::Kind dkind;
    
    const char *text;
    int rc;
    while ((rc = sqlite3_step( _dependency_handle)) == SQLITE_ROW) {
	try {
	    dep_type = (RCDependencyType)sqlite3_column_int( _dependency_handle, 0);
	    name = string ( (const char *)sqlite3_column_text( _dependency_handle, 1) );
	    text = (const char *)sqlite3_column_text( _dependency_handle, 2);
	    dkind = target2kind( (RCDependencyTarget)sqlite3_column_int( _dependency_handle, 7 ) );

	    if (text == NULL) {
		cap = factory.parse( dkind, name );
	    }
	    else {
		version = text;
	        text = (const char *)sqlite3_column_text( _dependency_handle, 3);
		if (text != NULL)
		    release = text;
		else
		    release.clear();
		epoch = sqlite3_column_int( _dependency_handle, 4 );
		arch = DbAccess::Rc2Arch( (RCArch) sqlite3_column_int( _dependency_handle, 5 ) );
		rel = DbAccess::Rc2Rel( (RCResolvableRelation) sqlite3_column_int( _dependency_handle, 6 ) );

		cap = factory.parse( dkind, name, rel, Edition( version, release, epoch ) );
	    }

	    switch (dep_type) {
		case RC_DEP_TYPE_REQUIRE:
		    deps[Dep::REQUIRES].insert( cap );
		break;
		case RC_DEP_TYPE_PROVIDE:
		    deps[Dep::PROVIDES].insert( cap );
		break;
		case RC_DEP_TYPE_CONFLICT:
		    deps[Dep::CONFLICTS].insert( cap );
		break;
		case RC_DEP_TYPE_OBSOLETE:
		    deps[Dep::OBSOLETES].insert( cap );
		break;
		case RC_DEP_TYPE_PREREQUIRE:
		    deps[Dep::PREREQUIRES].insert( cap );
		break;
		case RC_DEP_TYPE_FRESHEN:
		    deps[Dep::FRESHENS].insert( cap );
		break;
		case RC_DEP_TYPE_RECOMMEND:
		    deps[Dep::RECOMMENDS].insert( cap );
		break;
		case RC_DEP_TYPE_SUGGEST:
		    deps[Dep::SUGGESTS].insert( cap );
		break;
		case RC_DEP_TYPE_SUPPLEMENT:
		    deps[Dep::SUPPLEMENTS].insert( cap );
		break;
		case RC_DEP_TYPE_ENHANCE:
		    deps[Dep::ENHANCES].insert( cap );
		break;
		default:
		   ERR << "Unhandled dep_type " << dep_type << endl;
		break;
	    }
	}
	catch ( Exception & excpt_r ) {
	    ERR << "Can't parse dependencies for resolvable_id " << resolvable_id << ", name '" << name << "', version '" << version << "', release '" << release << "'" << endl;
	    ZYPP_CAUGHT( excpt_r );
	}
    }

    sqlite3_reset (_dependency_handle);
    return deps;
}

#if 0

Message::Ptr
DbSourceImpl::createMessage (const DbReader & parsed)
{
    try
    {
	detail::ResImplTraits<DbMessageImpl>::Ptr impl(new DbMessageImpl(_source, parsed));

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
DbSourceImpl::createScript (const DbReader & parsed)
{
    try
    {
	detail::ResImplTraits<DbScriptImpl>::Ptr impl(new DbScriptImpl(_source, parsed));

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
DbSourceImpl::createPatch (const DbReader & parsed)
{
    try
    {
	detail::ResImplTraits<DbPatchImpl>::Ptr impl(new DbPatchImpl(_source, parsed));

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
DbSourceImpl::createPattern (const DbReader & parsed)
{
    try
    {
	detail::ResImplTraits<DbPatternImpl>::Ptr impl(new DbPatternImpl(_source, parsed));

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
DbSourceImpl::createProduct (const DbReader & parsed)
{
    try
    {
	detail::ResImplTraits<DbProductImpl>::Ptr impl(new DbProductImpl(_source, parsed));

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
DbSourceImpl::parserCallback (const DbReader & parsed)
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
#endif

//-----------------------------------------------------------------------------

