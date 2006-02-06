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
#include "DbAccess.h"

IMPL_PTR_TYPE(DbAccess);

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "DbAccess"

using namespace std;
using namespace zypp;

//----------------------------------------------------------------------------

// Convert ZYPP relation operator to ZMD int
static int
Rel2Int (Rel op)
{
#define RELATION_ANY 0
#define RELATION_EQUAL (1 << 0)
#define RELATION_LESS (1 << 1)
#define RELATION_GREATER (1 << 2)
#define RELATION_NONE (1 << 3)

   /* This enum is here so that gdb can give us pretty strings */

    typedef enum {
	RC_RELATION_INVALID            = -1,
	RC_RELATION_ANY                = RELATION_ANY,
	RC_RELATION_EQUAL              = RELATION_EQUAL,
	RC_RELATION_LESS               = RELATION_LESS,
	RC_RELATION_LESS_EQUAL         = RELATION_LESS | RELATION_EQUAL,
	RC_RELATION_GREATER            = RELATION_GREATER,
	RC_RELATION_GREATER_EQUAL      = RELATION_GREATER | RELATION_EQUAL,
	RC_RELATION_NOT_EQUAL          = RELATION_LESS | RELATION_GREATER,
	RC_RELATION_NONE               = RELATION_NONE,
    } RCResolvableRelation;

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

//----------------------------------------------------------------------------

// convert ZYPP architecture string to ZMD int

typedef enum {
    RC_ARCH_UNKNOWN = -1,
    RC_ARCH_NOARCH = 0,
    RC_ARCH_I386,
    RC_ARCH_I486,
    RC_ARCH_I586,
    RC_ARCH_I686,
    RC_ARCH_X86_64,
    RC_ARCH_IA32E,
    RC_ARCH_ATHLON,
    RC_ARCH_PPC,
    RC_ARCH_PPC64,
    RC_ARCH_S390,
    RC_ARCH_S390X,
    RC_ARCH_IA64,
    RC_ARCH_SPARC,
    RC_ARCH_SPARC64,
} RCArch;

static int
arch2zmd (string str)
{
    struct archrc {
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

    struct archrc *aptr = archtable;
    while (aptr->arch != NULL) {
	break;
	aptr++;
    }

    return aptr->rc;
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
  /** Ctor */
DbAccess::DbAccess( const std::string & dbfile_r )
    : _dbfile( dbfile_r )
    , _db( NULL )
    , _insert_res_handle( NULL )
    , _insert_pkg_handle( NULL )
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


bool
DbAccess::openDb(bool for_writing)
{
    MIL << "DbAccess::openDb(" << (for_writing?"write":"read") << ")" << endl;

    if (_db) {
	WAR << "Db already open" << endl;
	return true;
    }

    int rc = sqlite3_open (_dbfile.c_str(), &_db);
    if (rc != SQLITE_OK) {
	ERR << "Can not open SQL database: " << sqlite3_errmsg (_db) << endl;
	return false;
    }

    MIL << "begin" << endl;
    sqlite3_exec (_db, "BEGIN", NULL, NULL, NULL);

    return true;
}

void
DbAccess::closeDb(void)
{
    MIL << "DbAccess::closeDb()" << endl;

    commit();

    if (_insert_res_handle) {
	sqlite3_finalize (_insert_res_handle);
	_insert_res_handle = NULL;
    }
    if (_insert_pkg_handle) {
	sqlite3_finalize (_insert_pkg_handle);
	_insert_pkg_handle = NULL;
    }
    if (_insert_dep_handle) {
	sqlite3_finalize (_insert_dep_handle);
	_insert_dep_handle = NULL;
    }
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
static sqlite3_stmt *
prepare_res_insert (sqlite3 *db)
{
    int rc;
    sqlite3_stmt *handle = NULL;

    string query (
        "INSERT INTO resolvables (name, version, release, epoch, arch,"
        "                         file_size, installed_size, catalog,"
        "                         installed, local, install_only) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    rc = sqlite3_prepare (db, query.c_str(), -1, &handle, NULL);

    if (rc != SQLITE_OK) {
	ERR << "Can not prepare resolvable insertion clause: " << sqlite3_errmsg (db) << endl;
        sqlite3_finalize (handle);
        handle = NULL;
    }

    return handle;
}


static sqlite3_stmt *
prepare_pkg_insert (sqlite3 *db)
{
    int rc;
    sqlite3_stmt *handle = NULL;

    string query (
        "INSERT INTO package_details (resolvable_id, section, summary, "
        "                             description, package_filename,"
        "                             signature_filename) "
        "VALUES (?, ?, ?, ?, ?, ?)"
	"");

    rc = sqlite3_prepare (db, query.c_str(), -1, &handle, NULL);

    if (rc != SQLITE_OK) {
	ERR << "Can not prepare package insertion clause: " << sqlite3_errmsg (db) << endl;
	sqlite3_finalize (handle);
	handle = NULL;
    }

    return handle;
}


static sqlite3_stmt *
prepare_dep_insert (sqlite3 *db)
{
    int rc;
    sqlite3_stmt *handle = NULL;

    string query (
	"INSERT INTO dependencies "
	"  (resolvable_id, dep_type, name, version, release, epoch, arch, relation) "
	"VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

    rc = sqlite3_prepare (db, query.c_str(), -1, &handle, NULL);

    if (rc != SQLITE_OK) {
	ERR << "Can not prepare dependency insertion clause: " << sqlite3_errmsg (db) << endl;
	sqlite3_finalize (handle);
	handle = NULL;
    }

    return handle;
}

bool
DbAccess::prepareWrite(void)
{
    MIL << "DbAccess::prepareWrite()" << endl;

    bool result = openDb(true);
    if (!result) return false;

    sqlite3_exec (_db, "PRAGMA synchronous = 0", NULL, NULL, NULL);

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


bool
DbAccess::prepareRead(void)
{
    return openDb(false);
}

//----------------------------------------------------------------------------

void
DbAccess::writeDependency( sqlite_int64 res_id, RCDependencyType type, const zypp::CapSet & capabilities)
{
    MIL << "DbAccess::writeDependency(" << res_id << ", " << type << ", ...)" << endl;

    int rc;
    sqlite3_stmt *handle = _insert_dep_handle;

    if (capabilities.empty())
	return;

    for (zypp::CapSet::const_iterator iter = capabilities.begin(); iter != capabilities.end(); ++iter) {

	sqlite3_bind_int64 (handle, 1, res_id);
	sqlite3_bind_int (handle, 2, type);
	sqlite3_bind_text (handle, 3, iter->index().c_str(), -1, SQLITE_STATIC);

	Edition edition = iter->edition();

	if (edition != Edition::noedition) {
	    sqlite3_bind_text (handle, 4, edition.version().c_str(), -1, SQLITE_STATIC);
	    sqlite3_bind_text (handle, 5, edition.release().c_str(), -1, SQLITE_STATIC);
	    Edition::epoch_t epoch = edition.epoch();
	    if (epoch != Edition::noepoch) {
		sqlite3_bind_int (handle, 6, epoch);
	    }
	    else {
		sqlite3_bind_int (handle, 6, 0);
	    }
	}
	else {
	    sqlite3_bind_int (handle, 6, 0);
	}

	sqlite3_bind_int (handle, 7, 0);				// arch
	sqlite3_bind_int (handle, 8, Rel2Int(iter->op()));

	rc = sqlite3_step (handle);
	sqlite3_reset (handle);

	if (rc != SQLITE_DONE) {
	    ERR << "Error adding package to SQL: " << sqlite3_errmsg (_db) << endl;
	}
    }
    return;
}


void
DbAccess::writeDependencies(sqlite_int64 id, Resolvable::constPtr res)
{
    MIL << "DbAccess::writeDependencies(" << id << ", " << *res << ")" << endl;

    writeDependency( id, RC_DEP_TYPE_REQUIRE, res->dep (Dep::REQUIRES));
    writeDependency( id, RC_DEP_TYPE_PROVIDE, res->dep (Dep::PROVIDES));
    writeDependency( id, RC_DEP_TYPE_CONFLICT, res->dep (Dep::CONFLICTS));
    writeDependency( id, RC_DEP_TYPE_OBSOLETE, res->dep (Dep::OBSOLETES));
    writeDependency( id, RC_DEP_TYPE_PREREQUIRE, res->dep (Dep::PREREQUIRES));
    writeDependency( id, RC_DEP_TYPE_FRESHEN, res->dep (Dep::FRESHENS));
    writeDependency( id, RC_DEP_TYPE_RECOMMEND, res->dep (Dep::RECOMMENDS));
    writeDependency( id, RC_DEP_TYPE_SUGGEST, res->dep (Dep::SUGGESTS));
    writeDependency( id, RC_DEP_TYPE_ENHANCE, res->dep (Dep::ENHANCES));
}


sqlite_int64
DbAccess::writePackage (sqlite_int64 id, Package::constPtr pkg)
{
    MIL <<  "DbAccess::writePackage(" << id << ", " << *pkg << ")" << endl;
    int rc;
    sqlite3_stmt *handle = _insert_pkg_handle;

    sqlite3_bind_int64 (handle, 1, id);
    sqlite3_bind_int (handle, 2, 1);							// FIXME section is an INTEGER ?!
    sqlite3_bind_text (handle, 3, pkg->summary().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text (handle, 4, desc2str(pkg->description()).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text (handle, 5, pkg->plainRpm().asString().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text (handle, 6, "signature_filename", -1, SQLITE_STATIC);		// FIXME
//    sqlite3_bind_text (handle, 6, pkg->signature_filename, -1, SQLITE_STATIC);

    rc = sqlite3_step (handle);
    sqlite3_reset (handle);

    if (rc != SQLITE_DONE) {
	ERR << "Error adding package to SQL: " << sqlite3_errmsg (_db) << endl;
	return -1;
    }
    sqlite_int64 rowid = sqlite3_last_insert_rowid (_db);

    return rowid;
}


sqlite_int64
DbAccess::writeResObject (ResObject::constPtr obj, bool is_installed)
{
    MIL << "DbAccess::writeResObject (" << *obj << ", " << (is_installed?"installed":"uninstalled") << ")" << endl;

    Resolvable::constPtr res = obj;
    Package::constPtr pkg = asKind<Package>(res);

    int rc;
    sqlite3_stmt *handle = _insert_res_handle;

    sqlite3_bind_text (handle, 1, obj->name().c_str(), -1, SQLITE_STATIC);
    Edition ed = obj->edition();
    sqlite3_bind_text (handle, 2, ed.version().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text (handle, 3, ed.release().c_str(), -1, SQLITE_STATIC);
    if (ed.epoch() == Edition::noepoch) {
	sqlite3_bind_int (handle, 4, 0);
    } else {
	sqlite3_bind_int (handle, 4, ed.epoch());
    }

    sqlite3_bind_int (handle, 5, arch2zmd (obj->arch().asString()));
    if (pkg != NULL) {
	sqlite3_bind_int64 (handle, 7, pkg->archivesize());
	sqlite3_bind_int64 (handle, 8, pkg->size());
    }
    else {
	sqlite3_bind_int64 (handle, 7, 0);
	sqlite3_bind_int64 (handle, 8, 0);
    }
    sqlite3_bind_text (handle, 9, obj->source().alias().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int (handle, 15, is_installed ? 1 : 0);		//pkg->installed
    sqlite3_bind_int (handle, 16, 0);					//pkg->local_package
    if (pkg != NULL) {
	sqlite3_bind_int (handle, 17, pkg->installOnly() ? 1 : 0);
    }
    else {
	sqlite3_bind_int (handle, 17, 0);
    }

    rc = sqlite3_step (handle);
    sqlite3_reset (handle);

    if (rc != SQLITE_DONE) {
	ERR << "Error adding package to SQL: " << sqlite3_errmsg (_db) << endl;
	return -1;
    }
    sqlite_int64 rowid = sqlite3_last_insert_rowid (_db);

    if (pkg != NULL)
	writePackage (rowid, pkg);

    writeDependencies (rowid, obj);

    return rowid;
}


//----------------------------------------------------------------------------
void
DbAccess::writeStore( const zypp::ResStore & store, bool is_installed )
{
    MIL << "DbAccess::writeStore()" << endl;

    if (store.empty()) {
	ERR << "Couldn't access the packaging system:" << endl;
	return;
    }

    for (ResStore::const_iterator iter = store.begin(); iter != store.end(); ++iter) {
	if (!writeResObject( *iter, is_installed))
	    break;
    }

    return;
}

void
DbAccess::writeResObjects( const ResObjectList & resolvables, bool is_installed )
{
    MIL << "DbAccess::writeResObjects()" << endl;

    prepareWrite();

    for (ResObjectList::const_iterator iter = resolvables.begin(); iter != resolvables.end(); ++iter) {
	if (!writeResObject( *iter, is_installed)) {
	    break;
	}
    }

    commit();

    return;
}
