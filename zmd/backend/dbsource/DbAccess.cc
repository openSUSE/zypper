/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/DbAccess.cc
 *
*/

#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/ZYppFactory.h"
#include "DbAccess.h"

IMPL_PTR_TYPE(DbAccess);

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "DbAccess"

using namespace std;
using namespace zypp;


static struct archrc {
	char *arch;
	RCArch rc;
    } archtable[] = {
	{ "noarch",	RC_ARCH_NOARCH },
	{ "i386",	RC_ARCH_I386 },
	{ "i486",	RC_ARCH_I486 },
	{ "i586",	RC_ARCH_I586 },
	{ "i686",	RC_ARCH_I686 },
	{ "x86_64",	RC_ARCH_X86_64 },
	{ "ia32e",	RC_ARCH_IA32E },
	{ "athlon",	RC_ARCH_ATHLON },
	{ "ppc",	RC_ARCH_PPC },
	{ "ppc64",	RC_ARCH_PPC64 },
	{ "s390",	RC_ARCH_S390 },
	{ "s390x",	RC_ARCH_S390X },
	{ "ia64",	RC_ARCH_IA64 },
	{ "sparc",	RC_ARCH_SPARC },
	{ "sparc64",	RC_ARCH_SPARC64 },
	{ NULL,		RC_ARCH_UNKNOWN }
    };


//----------------------------------------------------------------------------

// Convert ZYPP relation operator to ZMD RCResolvableRelation

RCResolvableRelation
DbAccess::Rel2Rc (Rel op)
{

   /* This enum is here so that gdb can give us pretty strings */

    switch (op.inSwitch()) {
	case Rel::EQ_e:	   return RC_RELATION_EQUAL; break;
	case Rel::NE_e:    return RC_RELATION_NOT_EQUAL; break;
	case Rel::LT_e:    return RC_RELATION_LESS; break;
	case Rel::LE_e:    return RC_RELATION_LESS_EQUAL; break;
	case Rel::GT_e:    return RC_RELATION_GREATER; break;
	case Rel::GE_e:    return RC_RELATION_GREATER_EQUAL; break;
	case Rel::ANY_e:   return RC_RELATION_ANY; break;
	case Rel::NONE_e:  return RC_RELATION_NONE; break;
    }
    return RC_RELATION_INVALID;
}


Rel
DbAccess::Rc2Rel (RCResolvableRelation rel)
{
    switch (rel) {
	case RC_RELATION_INVALID:	return Rel::NONE; break;
	case RC_RELATION_ANY:		return Rel::ANY; break;
	case RC_RELATION_EQUAL:		return Rel::EQ; break;
	case RC_RELATION_LESS:		return Rel::LT; break;
	case RC_RELATION_LESS_EQUAL:	return Rel::LE; break;
	case RC_RELATION_GREATER:	return Rel::GT; break;
	case RC_RELATION_GREATER_EQUAL:	return Rel::GE; break;
	case RC_RELATION_NOT_EQUAL:	return Rel::NE; break;
	case RC_RELATION_NONE:		return Rel::ANY; break;
    }
    return Rel::NONE;
}

//----------------------------------------------------------------------------

// convert ZYPP architecture string to ZMD int

RCArch
DbAccess::Arch2Rc (const Arch & arch)
{
    string arch_str = arch.asString();
    struct archrc *aptr = archtable;
    while (aptr->arch != NULL) {
	if (arch_str == aptr->arch)
	    break;
	aptr++;
    }

    return aptr->rc;
}

Arch
DbAccess::Rc2Arch (RCArch rc)
{
    if (rc == RC_ARCH_UNKNOWN)
	return Arch();

    struct archrc *aptr = archtable;
    while (aptr->arch != NULL) {
	if (aptr->rc == rc) {
	    return Arch (aptr->arch);
	}
	aptr++;
    }
    WAR << "DbAccess::Rc2Arch(" << rc << ") unknown" << endl;
    return Arch ();
}


// convert description Text (list<string>) to a single string
static string
desc2str (const Text t)
{
    static string s;		// static so we can use sqlite STATIC below
    s.clear();
    string::size_type authors = t.find ("Authors:");		// strip off 'Authors:'
    if (authors != string::npos) {
	do {
	    --authors;
	} while (t[authors] == ' ' || t[authors] == '\n');
    }
    s = string (t, 0, authors);
    return s;
}

//----------------------------------------------------------------------------
// convert ZYPP Resolvable kind to ZMD RCDependencyTarget

static RCDependencyTarget
kind2target( Resolvable::Kind kind )
{
    if (kind == ResTraits<Package>::kind)	 return RC_DEP_TARGET_PACKAGE;
    else if (kind == ResTraits<Script>::kind)	 return RC_DEP_TARGET_SCRIPT;
    else if (kind == ResTraits<Message>::kind)	 return RC_DEP_TARGET_MESSAGE;
    else if (kind == ResTraits<Patch>::kind)	 return RC_DEP_TARGET_PATCH;
    else if (kind == ResTraits<Selection>::kind) return RC_DEP_TARGET_SELECTION;
    else if (kind == ResTraits<Pattern>::kind)	 return RC_DEP_TARGET_PATTERN;
    else if (kind == ResTraits<Product>::kind)	 return RC_DEP_TARGET_PRODUCT;
    else if (kind == ResTraits<Language>::kind)	 return RC_DEP_TARGET_LANGUAGE;
    else if (kind == ResTraits<Atom>::kind)	 return RC_DEP_TARGET_ATOM;
    else if (kind == ResTraits<SrcPackage>::kind) return RC_DEP_TARGET_SRC;

    WAR << "Unknown resolvable kind " << kind << endl;
    return RC_DEP_TARGET_UNKNOWN;
}

//----------------------------------------------------------------------------
// convert ZYPP ResStatus to ZMD RCResolvableStatus

static RCResolvableStatus
resstatus2rcstatus( ResStatus status )
{
    if (status.isUnneeded()) return RC_RES_STATUS_UNNEEDED;
    else if (status.isSatisfied()
	     || status.isComplete()) return RC_RES_STATUS_SATISFIED;
    else if (status.isIncomplete()
	     || status.isNeeded()) return RC_RES_STATUS_BROKEN;
    return RC_RES_STATUS_UNDETERMINED;
}

//----------------------------------------------------------------------------

  /** Ctor */
DbAccess::DbAccess( const std::string & dbfile_r )
    : _dbfile( dbfile_r )
    , _db( NULL )
    , _insert_res_handle( NULL )
    , _insert_pkg_handle( NULL )
    , _insert_patch_handle( NULL )
    , _insert_selection_handle( NULL )
    , _insert_pattern_handle( NULL )
    , _insert_product_handle( NULL )
    , _insert_dep_handle( NULL )
{
    MIL << "DbAccess::DbAccess(" << dbfile_r << ")" << endl;
}


  /** Dtor */
DbAccess::~DbAccess( )
{
    closeDb();
}


std::ostream &
DbAccess::dumpOn( std::ostream & str ) const
{
    str << "DbAccess(" << _dbfile << ")" << endl;
    return str;
}


//----------------------------------------------------------------------------

static sqlite3_stmt *
prepare_handle( sqlite3 *db, const string & query )
{
    int rc;
    sqlite3_stmt *handle = NULL;

    rc = sqlite3_prepare (db, query.c_str(), -1, &handle, NULL);

    if (rc != SQLITE_OK) {
	ERR << "Can not prepare '" << query << "': " << sqlite3_errmsg (db) << endl;
        sqlite3_finalize( handle);
        handle = NULL;
    }

    return handle;
}


static sqlite3_stmt *
prepare_res_insert (sqlite3 *db)
{
    string query (
	//			  1     2        3        4      5
        "INSERT INTO resolvables (name, version, release, epoch, arch,"
	//			  6               7
        "                         installed_size, catalog,"
	//			  8          9
        "                         installed, local) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

    return prepare_handle( db, query );
}


static sqlite3_stmt *
prepare_pkg_insert (sqlite3 *db)
{
    string query (
	//			      1              2          3
        "INSERT INTO package_details (resolvable_id, rpm_group, summary, "
	//			      4            5		     6
        "                             description, package_filename, signature_filename,"
 	//			      7          8
        "                             file_size, install_only) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
	"");

    return prepare_handle( db, query );
}


static sqlite3_stmt *
prepare_patch_insert (sqlite3 *db)
{
    string query (
	//			    1              2         3
        "INSERT INTO patch_details (resolvable_id, patch_id, status, "
	//			    4              5
        "                           creation_time, category,"
	//			    6       7        8
        "                           reboot, restart, interactive) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
	"");

    return prepare_handle( db, query );
}


static sqlite3_stmt *
prepare_selection_insert (sqlite3 *db)
{
    string query (
	//			        1              2
//        "INSERT INTO selection_details (resolvable_id, status) "
        "INSERT INTO pattern_details (resolvable_id, status) "
        "VALUES (?, ?)"
	"");

    return prepare_handle( db, query );
}


static sqlite3_stmt *
prepare_pattern_insert (sqlite3 *db)
{
    string query (
	//			      1              2
        "INSERT INTO pattern_details (resolvable_id, status) "
        "VALUES (?, ?)"
	"");

    return prepare_handle( db, query );
}


static sqlite3_stmt *
prepare_product_insert (sqlite3 *db)
{
    string query (
	//			      1              2       3
        "INSERT INTO product_details (resolvable_id, status, category) "
        "VALUES (?, ?, ?)"
	"");

    return prepare_handle( db, query );
}


static sqlite3_stmt *
prepare_dep_insert (sqlite3 *db)
{
    string query (
	"INSERT INTO dependencies "
	//  1              2         3     4        5        6      7     8         9
	"  (resolvable_id, dep_type, name, version, release, epoch, arch, relation, dep_target) "
	"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

    return prepare_handle( db, query );
}


bool
DbAccess::prepareWrite(void)
{
    XXX << "DbAccess::prepareWrite()" << endl;

    sqlite3_exec (_db, "PRAGMA synchronous = 0", NULL, NULL, NULL);

    bool result = true;

    _insert_res_handle = prepare_res_insert (_db);
    if (_insert_res_handle == NULL) {
	result = false;
	goto cleanup;
    }

    _insert_pkg_handle = prepare_pkg_insert (_db);
    if (_insert_pkg_handle == NULL) {
	result = false;
	goto cleanup;
    }

    _insert_patch_handle = prepare_patch_insert (_db);
    if (_insert_patch_handle == NULL) {
	result = false;
	goto cleanup;
    }
#if 0
    _insert_selection_handle = prepare_selection_insert (_db);
    if (_insert_selection_handle == NULL) {
	result = false;
	goto cleanup;
    }
#endif
    _insert_pattern_handle = prepare_pattern_insert (_db);
    if (_insert_pattern_handle == NULL) {
	result = false;
	goto cleanup;
    }

    _insert_product_handle = prepare_product_insert (_db);
    if (_insert_product_handle == NULL) {
	result = false;
	goto cleanup;
    }

    _insert_dep_handle = prepare_dep_insert (_db);
    if (_insert_dep_handle == NULL) {
	result = false;
	goto cleanup;
    }

 cleanup:
    if (result == false) {
	closeDb();
    }

    return true;
}


//----------------------------------------------------------------------------

bool
DbAccess::openDb( bool for_writing )
{
    XXX << "DbAccess::openDb(" << (for_writing?"write":"read") << ")" << endl;

    if (_db) {
	WAR << "Db already open" << endl;
	return true;
    }

    int rc = sqlite3_open (_dbfile.c_str(), &_db);
    if (rc != SQLITE_OK
	|| _db == NULL) {
	ERR << "Can not open SQL database: " << sqlite3_errmsg (_db) << endl;
	return false;
    }

    if (for_writing) {
	if (!prepareWrite())
	    return false;
    }

    sqlite3_exec (_db, "BEGIN", NULL, NULL, NULL);

    return true;
}


static void
close_handle( sqlite3_stmt **handle )
{
    if (*handle) {
	sqlite3_finalize (*handle);
	*handle = NULL;
    }
    return;
}


void
DbAccess::closeDb(void)
{
    XXX << "DbAccess::closeDb()" << endl;

    commit();

    close_handle( &_insert_res_handle );
    close_handle( &_insert_pkg_handle );
    close_handle( &_insert_patch_handle );
//    close_handle( &_insert_selection_handle );
    close_handle( &_insert_pattern_handle );
    close_handle( &_insert_product_handle );
    close_handle( &_insert_dep_handle );

    if (_db) {
	sqlite3_close (_db);
	_db = NULL;
    }
    return;
}


void
DbAccess::commit(void)
{
    if (_db)
	sqlite3_exec (_db, "COMMIT", NULL, NULL, NULL);
}

//----------------------------------------------------------------------------
// dependency

void
DbAccess::writeDependency( sqlite_int64 res_id, RCDependencyType type, const zypp::CapSet & capabilities)
{
    XXX << "DbAccess::writeDependency(" << res_id << ", " << type << ", ...)" << endl;

    int rc;
    sqlite3_stmt *handle = _insert_dep_handle;

    if (capabilities.empty())
	return;

    for (zypp::CapSet::const_iterator iter = capabilities.begin(); iter != capabilities.end(); ++iter) {

	RCDependencyTarget refers = kind2target( iter->refers() );
	if (refers == RC_DEP_TARGET_UNKNOWN) continue;

	sqlite3_bind_int64( handle, 1, res_id);
	sqlite3_bind_int( handle, 2, type);
	sqlite3_bind_text( handle, 3, iter->index().c_str(), -1, SQLITE_STATIC );

	Edition edition = iter->edition();

	if (iter->op() != Rel::NONE
	    && iter->op() != Rel::ANY
	    && edition != Edition::noedition)
	{
	    sqlite3_bind_text( handle, 4, edition.version().c_str(), -1, SQLITE_STATIC );
	    sqlite3_bind_text( handle, 5, edition.release().c_str(), -1, SQLITE_STATIC );
	    Edition::epoch_t epoch = edition.epoch();
	    if (epoch != Edition::noepoch) {
		sqlite3_bind_int( handle, 6, epoch);
	    }
	    else {
		sqlite3_bind_int( handle, 6, 0);
	    }
	    sqlite3_bind_int( handle, 7, -1);				// arch
	    sqlite3_bind_int( handle, 8, Rel2Rc( iter->op() ));
	}
	else {
	    sqlite3_bind_text( handle, 4, NULL, -1, SQLITE_STATIC );
	    sqlite3_bind_text( handle, 5, NULL, -1, SQLITE_STATIC );
	    sqlite3_bind_int( handle, 6, 0);
	    sqlite3_bind_int( handle, 7, -1);				// arch
	    sqlite3_bind_int( handle, 8, RC_RELATION_NONE );
	}

	sqlite3_bind_int( handle, 9, refers );

	rc = sqlite3_step( handle);
	sqlite3_reset( handle);

	if (rc != SQLITE_DONE) {
	    ERR << "Error adding package to SQL: " << sqlite3_errmsg (_db) << endl;
	}
    }
    return;
}


void
DbAccess::writeDependencies(sqlite_int64 id, Resolvable::constPtr res)
{
    XXX << "DbAccess::writeDependencies(" << id << ", " << *res << ")" << endl;

    writeDependency( id, RC_DEP_TYPE_REQUIRE,	 res->dep (Dep::REQUIRES));
    writeDependency( id, RC_DEP_TYPE_PROVIDE,	 res->dep (Dep::PROVIDES));
    writeDependency( id, RC_DEP_TYPE_CONFLICT,	 res->dep (Dep::CONFLICTS));
    writeDependency( id, RC_DEP_TYPE_OBSOLETE,	 res->dep (Dep::OBSOLETES));
    writeDependency( id, RC_DEP_TYPE_PREREQUIRE, res->dep (Dep::PREREQUIRES));
    writeDependency( id, RC_DEP_TYPE_FRESHEN,	 res->dep (Dep::FRESHENS));
    writeDependency( id, RC_DEP_TYPE_RECOMMEND,  res->dep (Dep::RECOMMENDS));
    writeDependency( id, RC_DEP_TYPE_SUGGEST,	 res->dep (Dep::SUGGESTS));
    writeDependency( id, RC_DEP_TYPE_SUPPLEMENT, res->dep (Dep::SUPPLEMENTS));
    writeDependency( id, RC_DEP_TYPE_ENHANCE,	 res->dep (Dep::ENHANCES));
}


//----------------------------------------------------------------------------
// package

sqlite_int64
DbAccess::writePackage (sqlite_int64 id, Package::constPtr pkg, ResStatus status )
{
    XXX <<  "DbAccess::writePackage(" << id << ", " << *pkg << ")" << endl;
    int rc;
    sqlite3_stmt *handle = _insert_pkg_handle;

    sqlite3_bind_int64( handle, 1, id);
    sqlite3_bind_text( handle, 2, pkg->group().c_str(), -1, SQLITE_STATIC );
    sqlite3_bind_text( handle, 3, pkg->summary().c_str(), -1, SQLITE_STATIC );
    sqlite3_bind_text( handle, 4, desc2str(pkg->description()).c_str(), -1, SQLITE_STATIC );
    sqlite3_bind_text( handle, 5, pkg->plainRpm().asString().c_str(), -1, SQLITE_STATIC );
    sqlite3_bind_text( handle, 6, NULL, -1, SQLITE_STATIC );
    sqlite3_bind_int( handle, 7, pkg->size());
    sqlite3_bind_int( handle, 8, pkg->installOnly() ? 1 : 0);

    rc = sqlite3_step( handle);
    sqlite3_reset( handle);

    if (rc != SQLITE_DONE) {
	ERR << "Error adding package to SQL: " << sqlite3_errmsg (_db) << endl;
	return -1;
    }
    sqlite_int64 rowid = sqlite3_last_insert_rowid (_db);

    return rowid;
}


//----------------------------------------------------------------------------
// patch

sqlite_int64
DbAccess::writePatch (sqlite_int64 id, Patch::constPtr patch, ResStatus status )
{
    XXX <<  "DbAccess::writePatch(" << id << ", " << *patch << ")" << endl;
    int rc;
    sqlite3_stmt *handle = _insert_patch_handle;

    sqlite3_bind_int64( handle, 1, id );
    sqlite3_bind_text( handle, 2, patch->id().c_str(), -1, SQLITE_STATIC );
    sqlite3_bind_int( handle, 3, resstatus2rcstatus( status ) );
    sqlite3_bind_int64( handle, 4, patch->timestamp() );
    sqlite3_bind_text( handle, 5, patch->category().c_str(), -1, SQLITE_STATIC );
    sqlite3_bind_int( handle, 6, patch->reboot_needed() ? 1 : 0 );
    sqlite3_bind_int( handle, 7, patch->affects_pkg_manager() ? 1 : 0 );
    sqlite3_bind_int( handle, 8, patch->interactive() ? 1 : 0 );

    rc = sqlite3_step( handle);
    sqlite3_reset( handle);

    if (rc != SQLITE_DONE) {
	ERR << "Error adding patch to SQL: " << sqlite3_errmsg (_db) << endl;
	return -1;
    }
    sqlite_int64 rowid = sqlite3_last_insert_rowid (_db);

    return rowid;
}


//----------------------------------------------------------------------------
// selection

sqlite_int64
DbAccess::writeSelection( sqlite_int64 id, Selection::constPtr selection, ResStatus status )
{
    XXX <<  "DbAccess::writeSelection(" << id << ", " << *selection << ")" << endl;
    int rc;
    sqlite3_stmt *handle = _insert_selection_handle;

    sqlite3_bind_int64( handle, 1, id);
    sqlite3_bind_int( handle, 2, resstatus2rcstatus( status ) );

    rc = sqlite3_step( handle);
    sqlite3_reset( handle);

    if (rc != SQLITE_DONE) {
	ERR << "Error adding selection to SQL: " << sqlite3_errmsg (_db) << endl;
	return -1;
    }
    sqlite_int64 rowid = sqlite3_last_insert_rowid (_db);

    return rowid;
}


//----------------------------------------------------------------------------
// pattern

sqlite_int64
DbAccess::writePattern (sqlite_int64 id, Pattern::constPtr pattern, ResStatus status )
{
    XXX <<  "DbAccess::writePattern(" << id << ", " << *pattern << ")" << endl;
    int rc;
    sqlite3_stmt *handle = _insert_pattern_handle;

    sqlite3_bind_int64( handle, 1, id);
    sqlite3_bind_int( handle, 2, resstatus2rcstatus( status ) );

    rc = sqlite3_step( handle);
    sqlite3_reset( handle);

    if (rc != SQLITE_DONE) {
	ERR << "Error adding pattern to SQL: " << sqlite3_errmsg (_db) << endl;
	return -1;
    }
    sqlite_int64 rowid = sqlite3_last_insert_rowid (_db);

    return rowid;
}


//----------------------------------------------------------------------------
// product

sqlite_int64
DbAccess::writeProduct (sqlite_int64 id, Product::constPtr product, ResStatus status )
{
    XXX <<  "DbAccess::writeProduct(" << id << ", " << *product << ")" << endl;
    int rc;
    sqlite3_stmt *handle = _insert_product_handle;

    sqlite3_bind_int64( handle, 1, id);
    sqlite3_bind_int( handle, 2, resstatus2rcstatus( status ) );
    sqlite3_bind_text( handle, 3, product->category().c_str(), -1, SQLITE_STATIC );

    rc = sqlite3_step( handle);
    sqlite3_reset( handle);

    if (rc != SQLITE_DONE) {
	ERR << "Error adding product to SQL: " << sqlite3_errmsg (_db) << endl;
	return -1;
    }
    sqlite_int64 rowid = sqlite3_last_insert_rowid (_db);

    return rowid;
}


//----------------------------------------------------------------------------
// resolvable

sqlite_int64
DbAccess::writeResObject (ResObject::constPtr obj, ResStatus status, const char *catalog)
{
    XXX << "DbAccess::writeResObject (" << *obj << ", " << status << ")" << endl;

    Resolvable::constPtr res = obj;

    int rc;
    sqlite3_stmt *handle = _insert_res_handle;

    sqlite3_bind_text( handle, 1, obj->name().c_str(), -1, SQLITE_STATIC );
    Edition ed = obj->edition();
    sqlite3_bind_text( handle, 2, ed.version().c_str(), -1, SQLITE_STATIC );
    sqlite3_bind_text( handle, 3, ed.release().c_str(), -1, SQLITE_STATIC );
    if (ed.epoch() == Edition::noepoch) {
	sqlite3_bind_int( handle, 4, 0);
    } else {
	sqlite3_bind_int( handle, 4, ed.epoch());
    }

    sqlite3_bind_int( handle, 5, Arch2Rc (obj->arch()) );
    sqlite3_bind_int64( handle, 6, obj->size() );
    if (catalog != NULL)
	sqlite3_bind_text( handle, 7, catalog, -1, SQLITE_STATIC );
    else
	sqlite3_bind_text( handle, 7, obj->source().alias().c_str(), -1, SQLITE_STATIC );
    sqlite3_bind_int( handle,  8, status.isInstalled() ? 1 : 0);
    sqlite3_bind_int( handle, 9, 0);					//pkg->local_package

    rc = sqlite3_step( handle);
    sqlite3_reset( handle);

    if (rc != SQLITE_DONE) {
	ERR << "Error adding package to SQL: " << sqlite3_errmsg (_db) << endl;
	return -1;
    }
    sqlite_int64 rowid = sqlite3_last_insert_rowid (_db);

    Package::constPtr pkg = asKind<Package>(res);
    if (pkg != NULL) writePackage (rowid, pkg, status);
    else {
	Patch::constPtr patch = asKind<Patch>(res);
	if (patch != NULL) writePatch (rowid, patch, status);
	else {
	    Selection::constPtr selection = asKind<Selection>(res);
	    if (selection != NULL) writeSelection (rowid, selection, status);
	    else {
		Pattern::constPtr pattern = asKind<Pattern>(res);
		if (pattern != NULL) writePattern (rowid, pattern, status);
		else {
		    Product::constPtr product = asKind<Product>(res);
		    if (product != NULL) writeProduct (rowid, product, status);
		}
	    }
	}
    }

    writeDependencies (rowid, obj);

    return rowid;
}


//----------------------------------------------------------------------------
/** check if catalog exists */
bool
DbAccess::haveCatalog( const std::string & catalog )
{
    string query ("SELECT * FROM catalogs WHERE id = ? ");

    sqlite3_stmt *handle = prepare_handle( _db, query );
    if (handle == NULL) {
	return false;
    }

    sqlite3_bind_text( handle, 1, catalog.c_str(), -1, SQLITE_STATIC );

    int rc = sqlite3_step( handle);
    if (rc == SQLITE_ROW) {
	DBG << "Found catalog" << endl;
    }
    else if (rc != SQLITE_DONE) {
	ERR << "rc " << rc << ": " << sqlite3_errmsg (_db) << endl;
    }
    sqlite3_reset( handle);

    return (rc == SQLITE_ROW);
}


/** insert catalog */
bool
DbAccess::insertCatalog( const std::string & catalog, const string & name, const string & alias, const string & description )
{
    string query ("INSERT INTO catalogs(id,name,alias,description) VALUES (?,?,?,?) ");

    sqlite3_stmt *handle = prepare_handle( _db, query );
    if (handle == NULL) {
	return false;
    }

    sqlite3_bind_text( handle, 1, catalog.c_str(), -1, SQLITE_STATIC );
    sqlite3_bind_text( handle, 2, name.c_str(), -1, SQLITE_STATIC );
    sqlite3_bind_text( handle, 3, alias.c_str(), -1, SQLITE_STATIC );
    sqlite3_bind_text( handle, 4, description.c_str(), -1, SQLITE_STATIC );

    int rc = sqlite3_step( handle);
    if (rc != SQLITE_DONE) {
	ERR << "rc " << rc << "Error writing catalog: " << sqlite3_errmsg (_db) << endl;
    }
    sqlite3_reset( handle);

    return (rc == SQLITE_DONE);
}


/** remove catalog, remove all resolvables of this catalog */
bool
DbAccess::removeCatalog( const std::string & catalog )
{
    string query ("DELETE FROM catalogs where id = ? ");

    sqlite3_stmt *handle = prepare_handle( _db, query );
    if (handle == NULL) {
	return false;
    }

    sqlite3_bind_text( handle, 1, catalog.c_str(), -1, SQLITE_STATIC );

    int rc = sqlite3_step( handle);
    if (rc != SQLITE_DONE) {
	ERR << "rc " << rc << ", Error removing catalog: " << sqlite3_errmsg (_db) << endl;
    }
    sqlite3_reset( handle);

    return (rc == SQLITE_DONE);
}


//----------------------------------------------------------------------------
// store

void
DbAccess::writeStore( const zypp::ResStore & store, ResStatus status, const char *catalog )
{
    XXX << "DbAccess::writeStore()" << endl;

    if (store.empty()) {
	ERR << "Store is empty." << endl;
	return;
    }

    Arch sysarch = getZYpp()->architecture();

    int count = 0;
    sqlite_int64 rowid = 0;
    for (ResStore::const_iterator iter = store.begin(); iter != store.end(); ++iter) {
	ResObject::constPtr obj = *iter;
	if (!obj) {
	    WAR << "Huh ? No object ?" << endl;
	    continue;
	}

	if (obj->kind() != ResTraits<SrcPackage>::kind		// don't write src/nosrc packages
            && ( status == ResStatus::installed			// installed ones are ok
		|| obj->arch().compatibleWith( sysarch ) ) )	//   and so are architecturally compatible ones
	{
	    rowid = writeResObject( obj, status, catalog );
	    if (rowid < 0)
		break;
	    ++count;
	}
    }

    MIL << "Wrote " << count << " resolvables to database, last rowid " << rowid << endl;
    return;
}

//----------------------------------------------------------------------------
// pool

void
DbAccess::writePool( const zypp::ResPool & pool, const char *catalog )
{
    XXX << "DbAccess::writePool()" << endl;

    if (pool.empty()) {
	ERR << "Pool is empty" << endl;
	return;
    }

    int count = 0;
    for (ResPool::const_iterator iter = pool.begin(); iter != pool.end(); ++iter) {
	if (!writeResObject( iter->resolvable(), iter->status(), catalog ))
	    break;
	++count;
    }

    MIL << "Wrote " << count << " resolvables to database" << endl;
    return;
}


