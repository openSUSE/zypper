/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* SqliteSourceImpl.cc
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

#include "SqliteSourceImpl.h"

#include "SqlitePackageImpl.h"
#include "SqliteAtomImpl.h"
#include "SqliteMessageImpl.h"
#include "SqliteScriptImpl.h"
#include "SqliteLanguageImpl.h"
#include "SqlitePatchImpl.h"
#include "SqlitePatternImpl.h"
#include "SqliteProductImpl.h"

#include "zypp/source/SourceImpl.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/CapFactory.h"

using namespace std;
using namespace zypp;

//---------------------------------------------------------------------------

SqliteSourceImpl::SqliteSourceImpl()
    : _db (NULL)
    , _idmap (NULL)
{
}


void
SqliteSourceImpl::factoryInit()
{
    MIL << "SqliteSourceImpl::factoryInit()" << endl;

    try {
	media::MediaManager media_mgr;
	MIL << "Adding no media verifier" << endl;
	media::MediaAccessId _media = _media_set->getMediaAccessId(1);
	media_mgr.delVerifier( _media);
	media_mgr.addVerifier( _media, media::MediaVerifierRef( new media::NoVerifier()));
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
SqliteSourceImpl::attachDatabase( sqlite3 *db)
{
    _db = db;
}

void
SqliteSourceImpl::attachIdMap (IdMap *idmap)
{
  _idmap = idmap;
}

void
SqliteSourceImpl::attachZyppSource(zypp::Source_Ref source)
{
  _zyppsource = source;
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

    rc = sqlite3_prepare ( db, query, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not prepare dependency selection clause: " << sqlite3_errmsg ( db) << endl;
	sqlite3_finalize (handle);
	return NULL;
    }

    return handle;
}


static sqlite3_stmt *
create_resolvables_handle (sqlite3 *db)
{
    const char *query;
    int rc;
    sqlite3_stmt *handle = NULL;

    query =
	//      0   1     2        3        4      5
	"SELECT id, name, version, release, epoch, arch, "
	//      6               7        8          9      10
	"       installed_size, catalog, installed, local, kind "
	"FROM resolvables "
	"WHERE catalog = ? AND kind = ?";

    rc = sqlite3_prepare ( db, query, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not prepare resolvables selection clause: " << sqlite3_errmsg ( db) << endl;
	sqlite3_finalize (handle);
	return NULL;
    }

    return handle;
}


static sqlite3_stmt *
create_message_handle (sqlite3 *db)
{
    const char *query;
    int rc;
    sqlite3_stmt *handle = NULL;

    query =
	//      0   1     2        3        4      5
	"SELECT id, name, version, release, epoch, arch, "
	//      6               7
	"       installed_size, catalog,"
	//      8          9      10
	"       installed, local, content "
	"FROM messages "
	"WHERE catalog = ?";

    rc = sqlite3_prepare ( db, query, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not prepare messages selection clause: " << sqlite3_errmsg ( db) << endl;
	sqlite3_finalize (handle);
	return NULL;
    }

    return handle;
}


static sqlite3_stmt *
create_script_handle (sqlite3 *db)
{
    const char *query;
    int rc;
    sqlite3_stmt *handle = NULL;

    query =
	//      0   1     2        3        4      5
	"SELECT id, name, version, release, epoch, arch, "
	//      6               7
	"       installed_size, catalog,"
	//      8          9      10	     11
	"       installed, local, do_script, undo_script "
	"FROM scripts "
	"WHERE catalog = ?";

    rc = sqlite3_prepare ( db, query, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not prepare scripts selection clause: " << sqlite3_errmsg ( db) << endl;
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
	//      12       13           14           15
	"       summary, description, package_url, package_filename,"
	//      16            17
	"       install_only, media_nr "
	"FROM packages "
	"WHERE catalog = ?";

    rc = sqlite3_prepare ( db, query, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not prepare packages selection clause: " << sqlite3_errmsg ( db) << endl;
	sqlite3_finalize (handle);
	return NULL;
    }

    return handle;
}


static sqlite3_stmt *
create_patch_handle (sqlite3 *db)
{
    const char *query;
    int rc;
    sqlite3_stmt *handle = NULL;

    query =
	//      0   1     2        3        4      5
	"SELECT id, name, version, release, epoch, arch, "
	//      6               7
	"       installed_size, catalog,"
	//      8          9      10        11
	"       installed, local, patch_id, status,"
	//      12             13        14
	"       creation_time, category, reboot,"
	//      15       16
	"       restart, interactive "
	"FROM patches "
	"WHERE catalog = ?";

    rc = sqlite3_prepare ( db, query, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not prepare patches selection clause: " << sqlite3_errmsg ( db) << endl;
	ERR << "Clause: [" << query << "]" << endl;
	sqlite3_finalize (handle);
	return NULL;
    }

    return handle;
}


static sqlite3_stmt *
create_pattern_handle (sqlite3 *db)
{
    const char *query;
    int rc;
    sqlite3_stmt *handle = NULL;

    query =
	//      0   1     2        3        4      5
	"SELECT id, name, version, release, epoch, arch, "
	//      6               7
	"       installed_size, catalog,"
	//      8          9      10
	"       installed, local, status "
	"FROM patterns "
	"WHERE catalog = ?";

    rc = sqlite3_prepare ( db, query, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not prepare patterns selection clause: " << sqlite3_errmsg ( db) << endl;
	ERR << "Clause: [" << query << "]" << endl;
	sqlite3_finalize (handle);
	return NULL;
    }

    return handle;
}


static sqlite3_stmt *
create_product_handle (sqlite3 *db)
{
    const char *query;
    int rc;
    sqlite3_stmt *handle = NULL;

    query =
	//      0   1     2        3        4      5
	"SELECT id, name, version, release, epoch, arch, "
	//      6               7
	"       installed_size, catalog,"
	//      8          9      10      11
	"       installed, local, status, category "
	"FROM products "
	"WHERE catalog = ?";

    rc = sqlite3_prepare ( db, query, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not prepare products selection clause: " << sqlite3_errmsg ( db) << endl;
	ERR << "Clause: [" << query << "]" << endl;
	sqlite3_finalize (handle);
	return NULL;
    }

    return handle;
}


//-----------------------------------------------------------------------------

void
SqliteSourceImpl::createResolvables(Source_Ref source_r)
{
    MIL << "SqliteSourceImpl::createResolvables(" << source_r.id() << ")" << endl;
    _source = source_r;
    if ( _db == NULL) {
	ERR << "Must call attachDatabase() first" << endl;
	return;
    }

    _dependency_handle = create_dependency_handle ( _db);
    if ( _dependency_handle == NULL) return;

    createPackages();
    createAtoms();
    createMessages();
    createScripts();
    createLanguages();
    createPatches();
    createPatterns();
    createProducts();

    return;
}


void
SqliteSourceImpl::createAtoms(void)
{
    sqlite3_stmt *handle = create_resolvables_handle( _db );
    if (handle == NULL) return;

    sqlite3_bind_text (handle, 1, _source.id().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int (handle, 2, RC_DEP_TARGET_ATOM);

    int rc;
    while ((rc = sqlite3_step (handle)) == SQLITE_ROW) {

	string name;

	try
	{
	    sqlite_int64 id = sqlite3_column_int64( handle, 0 );
	    name = (const char *) sqlite3_column_text( handle, 1 );
	    string version ((const char *) sqlite3_column_text( handle, 2 ));
	    string release ((const char *) sqlite3_column_text( handle, 3 ));
	    unsigned epoch = sqlite3_column_int( handle, 4 );
	    Arch arch( SqliteAccess::Rc2Arch( (RCArch)(sqlite3_column_int( handle, 5 )) ) );

	    detail::ResImplTraits<SqliteAtomImpl>::Ptr impl( new SqliteAtomImpl( _source, id ) );

	    // Collect basic Resolvable data
	    NVRAD dataCollect( name,
			Edition( version, release, epoch ),
			arch,
			createDependencies (id ) );

	    Atom::Ptr atom = detail::makeResolvableFromImpl( dataCollect, impl );
	    _store.insert( atom );
	    XXX << "Atom[" << id << "] " << *atom << endl;
	    if ( _idmap != 0)
		(*_idmap)[id] = atom;
	}
	catch (const Exception & excpt_r)
	{
	    ERR << "Cannot create atom object '" << name << "' from catalog '" << _source.id() << "'" << endl;
	    sqlite3_reset (handle);
	    ZYPP_RETHROW (excpt_r);
	}
    }

    sqlite3_reset (handle);
    return;
}


void
SqliteSourceImpl::createMessages(void)
{
    sqlite3_stmt *handle = create_message_handle( _db );
    if (handle == NULL) return;

    sqlite3_bind_text( handle, 1, _source.id().c_str(), -1, SQLITE_STATIC );

    int rc;
    while ((rc = sqlite3_step (handle)) == SQLITE_ROW) {

	string name;

	try
	{
	    sqlite_int64 id = sqlite3_column_int64( handle, 0 );
	    name = (const char *) sqlite3_column_text( handle, 1 );
	    string version( (const char *)sqlite3_column_text( handle, 2 ) );
	    string release( (const char *)sqlite3_column_text( handle, 3 ) );
	    unsigned epoch = sqlite3_column_int( handle, 4 );
	    Arch arch( SqliteAccess::Rc2Arch( (RCArch)(sqlite3_column_int( handle, 5 )) ) );

	    string content( (const char *)sqlite3_column_text( handle, 10 ) );

	    detail::ResImplTraits<SqliteMessageImpl>::Ptr impl( new SqliteMessageImpl( _source, TranslatedText( content ), id ) );

	    // Collect basic Resolvable data
	    NVRAD dataCollect( name,
			Edition( version, release, epoch ),
			arch,
			createDependencies (id ) );

	    Message::Ptr message = detail::makeResolvableFromImpl( dataCollect, impl );
	    _store.insert( message );
	    XXX << "Message[" << id << "] " << *message << endl;
	    if (_idmap != 0)
		(*_idmap)[id] = message;
	}
	catch (const Exception & excpt_r)
	{
	    ERR << "Cannot create message object '" << name << "' from catalog '" << _source.id() << "'" << endl;
	    sqlite3_reset (handle);
	    ZYPP_RETHROW (excpt_r);
	}
    }

    sqlite3_reset (handle);
    return;
}


void
SqliteSourceImpl::createScripts(void)
{
    sqlite3_stmt *handle = create_script_handle( _db );
    if (handle == NULL) return;

    sqlite3_bind_text( handle, 1, _source.id().c_str(), -1, SQLITE_STATIC );

    int rc;
    while ((rc = sqlite3_step (handle)) == SQLITE_ROW) {

	string name;

	try
	{
	    sqlite_int64 id = sqlite3_column_int64( handle, 0 );
	    name = (const char *) sqlite3_column_text( handle, 1 );
	    string version ((const char *) sqlite3_column_text( handle, 2 ));
	    string release ((const char *) sqlite3_column_text( handle, 3 ));
	    unsigned epoch = sqlite3_column_int( handle, 4 );
	    Arch arch( SqliteAccess::Rc2Arch( (RCArch)(sqlite3_column_int( handle, 5 )) ) );

	    string do_script( (const char *)sqlite3_column_text( handle, 10 ) );
	    string undo_script( (const char *)sqlite3_column_text( handle, 11 ) );

	    detail::ResImplTraits<SqliteScriptImpl>::Ptr impl( new SqliteScriptImpl( _source, do_script, undo_script, id ) );

	    // Collect basic Resolvable data
	    NVRAD dataCollect( name,
			Edition( version, release, epoch ),
			arch,
			createDependencies (id ) );

	    Script::Ptr script = detail::makeResolvableFromImpl( dataCollect, impl );
	    _store.insert( script );
	    XXX << "Script[" << id << "] " << *script << endl;
	    if ( _idmap != 0)
		(*_idmap)[id] = script;
	}
	catch (const Exception & excpt_r)
	{
	    ERR << "Cannot create script object '" << name << "' from catalog '" << _source.id() << "'" << endl;
	    sqlite3_reset (handle);
	    ZYPP_RETHROW (excpt_r);
	}
    }

    sqlite3_reset (handle);
    return;
}


void
SqliteSourceImpl::createLanguages(void)
{
    sqlite3_stmt *handle = create_resolvables_handle( _db );
    if (handle == NULL) return;

    sqlite3_bind_text (handle, 1, _source.id().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int (handle, 2, RC_DEP_TARGET_LANGUAGE);

    int rc;
    while ((rc = sqlite3_step (handle)) == SQLITE_ROW) {

	string name;

	try
	{
	    sqlite_int64 id = sqlite3_column_int64( handle, 0 );
	    name = (const char *) sqlite3_column_text( handle, 1 );
	    string version ((const char *) sqlite3_column_text( handle, 2 ));
	    string release ((const char *) sqlite3_column_text( handle, 3 ));
	    unsigned epoch = sqlite3_column_int( handle, 4 );
	    Arch arch( SqliteAccess::Rc2Arch( (RCArch)(sqlite3_column_int( handle, 5 )) ) );

	    detail::ResImplTraits<SqliteLanguageImpl>::Ptr impl( new SqliteLanguageImpl( _source, id ) );

	    // Collect basic Resolvable data
	    NVRAD dataCollect( name,
			Edition( version, release, epoch ),
			arch,
			createDependencies (id ) );

	    Language::Ptr language = detail::makeResolvableFromImpl( dataCollect, impl );
	    _store.insert( language );
	    XXX << "Language[" << id << "] " << *language << endl;
	    if ( _idmap != 0)
		(*_idmap)[id] = language;
	}
	catch (const Exception & excpt_r)
	{
	    ERR << "Cannot create language object '" << name << "' from catalog '" << _source.id() << "'" << endl;
	    sqlite3_reset (handle);
	    ZYPP_RETHROW (excpt_r);
	}
    }

    sqlite3_reset (handle);
    return;
}


void
SqliteSourceImpl::createPackages(void)
{
    sqlite3_stmt *handle = create_package_handle ( _db);
    if (handle == NULL) return;

    sqlite3_bind_text (handle, 1, _source.id().c_str(), -1, SQLITE_STATIC);

    int rc;
    while ((rc = sqlite3_step (handle)) == SQLITE_ROW) {

	string name;

	try
	{
	    detail::ResImplTraits<SqlitePackageImpl>::Ptr impl( new SqlitePackageImpl( _zyppsource ? _zyppsource :_source ) );

	    sqlite_int64 id = sqlite3_column_int64( handle, 0 );
	    name = (const char *) sqlite3_column_text( handle, 1 );
	    string version ((const char *) sqlite3_column_text( handle, 2 ));
	    string release ((const char *) sqlite3_column_text( handle, 3 ));
	    unsigned epoch = sqlite3_column_int( handle, 4 );
	    Arch arch( SqliteAccess::Rc2Arch( (RCArch)(sqlite3_column_int( handle, 5 )) ) );

	    impl->readHandle( id, handle );

	    // Collect basic Resolvable data
	    NVRAD dataCollect( name,
			Edition( version, release, epoch ),
			arch,
			createDependencies (id ) );

	    Package::Ptr package = detail::makeResolvableFromImpl( dataCollect, impl );
	    _store.insert( package );
	    if ( _idmap != 0)
		(*_idmap)[id] = package;
	}
	catch (const Exception & excpt_r)
	{
	    ERR << "Cannot create package object '" << name << "' from catalog '" << _source.id() << "'" << endl;
	    sqlite3_reset (handle);
	    ZYPP_RETHROW (excpt_r);
	}
    }

    sqlite3_reset (handle);
    return;
}


void
SqliteSourceImpl::createPatches(void)
{
    sqlite3_stmt *handle = create_patch_handle( _db );
    if (handle == NULL) return;

    sqlite3_bind_text( handle, 1, _source.id().c_str(), -1, SQLITE_STATIC );

    int rc;
    while ((rc = sqlite3_step (handle)) == SQLITE_ROW) {

	string name;

	try
	{
	    detail::ResImplTraits<SqlitePatchImpl>::Ptr impl( new SqlitePatchImpl( _source ) );

	    sqlite_int64 id = sqlite3_column_int64( handle, 0 );
	    name = (const char *) sqlite3_column_text( handle, 1 );
	    string version ((const char *) sqlite3_column_text( handle, 2 ));
	    string release ((const char *) sqlite3_column_text( handle, 3 ));
	    unsigned epoch = sqlite3_column_int( handle, 4 );
	    Arch arch( SqliteAccess::Rc2Arch( (RCArch)(sqlite3_column_int( handle, 5 )) ) );

	    impl->readHandle( id, handle );

	    // Collect basic Resolvable data
	    NVRAD dataCollect( name,
			Edition( version, release, epoch ),
			arch,
			createDependencies (id ) );

	    Patch::Ptr patch = detail::makeResolvableFromImpl( dataCollect, impl );
	    _store.insert( patch );
	    XXX << "Patch[" << id << "] " << *patch << endl;
	    if ( _idmap != 0)
		(*_idmap)[id] = patch;
	}
	catch (const Exception & excpt_r)
	{
	    ERR << "Cannot create patch object '" << name << "' from catalog '" << _source.id() << "'" << endl;
	    sqlite3_reset (handle);
	    ZYPP_RETHROW (excpt_r);
	}
    }

    sqlite3_reset (handle);
    return;
}


void
SqliteSourceImpl::createPatterns(void)
{
    sqlite3_stmt *handle = create_pattern_handle( _db );
    if (handle == NULL) return;

    sqlite3_bind_text( handle, 1, _source.id().c_str(), -1, SQLITE_STATIC );

    int rc;
    while ((rc = sqlite3_step (handle)) == SQLITE_ROW) {

	string name;

	try
	{
	    detail::ResImplTraits<SqlitePatternImpl>::Ptr impl( new SqlitePatternImpl( _source ) );

	    sqlite_int64 id = sqlite3_column_int64( handle, 0 );
	    name = (const char *) sqlite3_column_text( handle, 1 );
	    string version ((const char *) sqlite3_column_text( handle, 2 ));
	    string release ((const char *) sqlite3_column_text( handle, 3 ));
	    unsigned epoch = sqlite3_column_int( handle, 4 );
	    Arch arch( SqliteAccess::Rc2Arch( (RCArch)(sqlite3_column_int( handle, 5 )) ) );

	    impl->readHandle( id, handle );

	    // Collect basic Resolvable data
	    NVRAD dataCollect( name,
			Edition( version, release, epoch ),
			arch,
			createDependencies (id ) );

	    Pattern::Ptr pattern = detail::makeResolvableFromImpl( dataCollect, impl );
	    _store.insert( pattern );
	    XXX << "Pattern[" << id << "] " << *pattern << endl;
	    if ( _idmap != 0)
		(*_idmap)[id] = pattern;
	}
	catch (const Exception & excpt_r)
	{
	    ERR << "Cannot create pattern object '" << name << "' from catalog '" << _source.id() << "'" << endl;
	    sqlite3_reset (handle);
	    ZYPP_RETHROW (excpt_r);
	}
    }

    sqlite3_reset (handle);
    return;
}


void
SqliteSourceImpl::createProducts(void)
{
    sqlite3_stmt *handle = create_product_handle( _db );
    if (handle == NULL) return;

    sqlite3_bind_text( handle, 1, _source.id().c_str(), -1, SQLITE_STATIC );

    int rc;
    while ((rc = sqlite3_step (handle)) == SQLITE_ROW) {

	string name;

	try
	{
	    detail::ResImplTraits<SqliteProductImpl>::Ptr impl( new SqliteProductImpl( _source ) );

	    sqlite_int64 id = sqlite3_column_int64( handle, 0 );
	    name = (const char *) sqlite3_column_text( handle, 1 );
	    string version ((const char *) sqlite3_column_text( handle, 2 ));
	    string release ((const char *) sqlite3_column_text( handle, 3 ));
	    unsigned epoch = sqlite3_column_int( handle, 4 );
	    Arch arch( SqliteAccess::Rc2Arch( (RCArch)(sqlite3_column_int( handle, 5 )) ) );

	    impl->readHandle( id, handle );

	    // Collect basic Resolvable data
	    NVRAD dataCollect( name,
			Edition( version, release, epoch ),
			arch,
			createDependencies (id ) );

	    Product::Ptr product = detail::makeResolvableFromImpl( dataCollect, impl );
	    _store.insert( product );
	    XXX << "Product[" << id << "] " << *product << endl;
	    if ( _idmap != 0)
		(*_idmap)[id] = product;
	}
	catch (const Exception & excpt_r)
	{
	    ERR << "Cannot create product object '" << name << "' from catalog '" << _source.id() << "'" << endl;
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

    switch ( dep_target)
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
	case RC_DEP_TARGET_LANGUAGE:	kind = ResTraits<Language>::kind;
	break;
	case RC_DEP_TARGET_ATOM:	kind = ResTraits<Atom>::kind;
	break;
	case RC_DEP_TARGET_SRC:		kind = ResTraits<SrcPackage>::kind;
	break;
	case RC_DEP_TARGET_SYSTEM:	kind = ResTraits<SystemResObject>::kind;
	break;
	default:			WAR << "Unknown dep_target " << dep_target << endl;
					kind = ResTraits<Package>::kind;
	break;
    }
    return kind;
}


Dependencies
SqliteSourceImpl::createDependencies (sqlite_int64 resolvable_id)
{
    Dependencies deps;
    CapFactory factory;

    sqlite3_bind_int64 ( _dependency_handle, 1, resolvable_id);

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
		arch = SqliteAccess::Rc2Arch( (RCArch) sqlite3_column_int( _dependency_handle, 5 ) );
		rel = SqliteAccess::Rc2Rel( (RCResolvableRelation) sqlite3_column_int( _dependency_handle, 6 ) );

		cap = factory.parse( dkind, name, rel, Edition( version, release, epoch ) );
	    }

	    switch ( dep_type) {
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

    sqlite3_reset ( _dependency_handle);
    return deps;
}

// EOF
