/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include "resolvable-writer.h"
#include <sqlite3.h>
#include <iostream>

#include "zypp/base/Exception.h"
#include "zypp/base/Logger.h"
#include "zypp/CapSet.h"
#include "zypp/Rel.h"
#include "zypp/Edition.h"
#include "zypp/Package.h"
#include "zypp/ResObject.h"
#include "zypp/Source.h"

#include <cstdlib>
#include <string>

using namespace zypp;
using namespace std;

using std::endl;
using std::string;

//----------------------------------------------------------------------------
typedef enum {
    RC_DEP_TYPE_REQUIRE = 0,
    RC_DEP_TYPE_PROVIDE,
    RC_DEP_TYPE_CONFLICT,
    RC_DEP_TYPE_OBSOLETE,
    RC_DEP_TYPE_PREREQUIRE,
    RC_DEP_TYPE_FRESHEN,
    RC_DEP_TYPE_RECOMMEND,
    RC_DEP_TYPE_SUGGEST,
    RC_DEP_TYPE_ENHANCE
} RCDependencyType;

typedef struct {
    sqlite3 *db;
    sqlite3_stmt *insert_res_handle;
    sqlite3_stmt *insert_pkg_handle;
    sqlite3_stmt *insert_dep_handle;
} RCDB;

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

//----------------------------------------------------------------------------- 

static void
rc_db_close (RCDB *rcdb)
{
    sqlite3_finalize (rcdb->insert_res_handle);
    sqlite3_finalize (rcdb->insert_pkg_handle);
    sqlite3_finalize (rcdb->insert_dep_handle);
    sqlite3_close (rcdb->db);
}


static RCDB *
rc_db_new (const string & path)
{
    RCDB *rcdb;
    sqlite3 *db = NULL;
    int rc;
    bool error = false;

    rcdb = new (RCDB);

    rc = sqlite3_open (path.c_str(), &db);
    if (rc != SQLITE_OK) {
	ERR << "Can not open SQL database: " << sqlite3_errmsg (db) << endl;
	error = true;
	goto cleanup;
    }

    rcdb->db = db;

    sqlite3_exec (rcdb->db, "PRAGMA synchronous = 0", NULL, NULL, NULL);
#if 0
    if (!create_tables (rcdb)) {
	error = true;
	goto cleanup;
    }
#endif

    rcdb->insert_res_handle = prepare_res_insert (db);
    if (rcdb->insert_res_handle == NULL) {
	error = true;
	goto cleanup;
    }

    rcdb->insert_pkg_handle = prepare_pkg_insert (db);
    if (rcdb->insert_pkg_handle == NULL) {
	error = true;
	goto cleanup;
    }

    rcdb->insert_dep_handle = prepare_dep_insert (db);
    if (rcdb->insert_dep_handle == NULL) {
	error = true;
	goto cleanup;
    }

 cleanup:
    if (error) {
	rc_db_close (rcdb);
	delete (rcdb);
	rcdb = NULL;
    }

    return rcdb;
}


static void
rc_db_begin (RCDB *rcdb)
{
    sqlite3_exec (rcdb->db, "BEGIN", NULL, NULL, NULL);
}


static void
rc_db_commit (RCDB *rcdb)
{
    sqlite3_exec (rcdb->db, "COMMIT", NULL, NULL, NULL);
}

//-----------------------------------------------------------------------------

static void
write_deps (RCDB *rcdb, sqlite_int64 pkg_id, int type, const zypp::CapSet & capabilities)
{
    int rc;
    sqlite3_stmt *handle = rcdb->insert_dep_handle;

    if (capabilities.empty())
	return;

    for (zypp::CapSet::const_iterator iter = capabilities.begin(); iter != capabilities.end(); ++iter) {
	
	sqlite3_bind_int64 (handle, 1, pkg_id);
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
	    ERR << "Error adding package to SQL: " << sqlite3_errmsg (rcdb->db) << endl;
	}
    }
}


static void
write_resolvable_deps (RCDB *rcdb, sqlite_int64 id, Resolvable::constPtr res)
{
    write_deps (rcdb, id, RC_DEP_TYPE_REQUIRE, res->dep (Dep::REQUIRES));
    write_deps (rcdb, id, RC_DEP_TYPE_PROVIDE, res->dep (Dep::PROVIDES));
    write_deps (rcdb, id, RC_DEP_TYPE_CONFLICT, res->dep (Dep::CONFLICTS));
    write_deps (rcdb, id, RC_DEP_TYPE_OBSOLETE, res->dep (Dep::OBSOLETES));
    write_deps (rcdb, id, RC_DEP_TYPE_PREREQUIRE, res->dep (Dep::PREREQUIRES));
    write_deps (rcdb, id, RC_DEP_TYPE_FRESHEN, res->dep (Dep::FRESHENS));
    write_deps (rcdb, id, RC_DEP_TYPE_RECOMMEND, res->dep (Dep::RECOMMENDS));
    write_deps (rcdb, id, RC_DEP_TYPE_SUGGEST, res->dep (Dep::SUGGESTS));
    write_deps (rcdb, id, RC_DEP_TYPE_ENHANCE, res->dep (Dep::ENHANCES));
}


static sqlite_int64
write_package (RCDB *rcdb, sqlite_int64 id, Package::constPtr pkg)
{
    int rc;
    sqlite3_stmt *handle = rcdb->insert_pkg_handle;

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
	ERR << "Error adding package to SQL: " << sqlite3_errmsg (rcdb->db) << endl;
	return -1;
    }
    sqlite_int64 rowid = sqlite3_last_insert_rowid (rcdb->db);
//fprintf (stderr, "%s-%s-%s : %lld\n", pkg->tag_name().c_str(),pkg->tag_version().c_str(),pkg->tag_release().c_str(), rowid);fflush(stderr);

    return rowid;
}


static sqlite_int64
write_resolvable (RCDB *rcdb, ResObject::constPtr obj, Package::constPtr pkg, bool is_installed)
{
    int rc;
    sqlite3_stmt *handle = rcdb->insert_res_handle;

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
	ERR << "Error adding package to SQL: " << sqlite3_errmsg (rcdb->db) << endl;
	return -1;
    }
    sqlite_int64 rowid = sqlite3_last_insert_rowid (rcdb->db);

    return rowid;
}


//-----------------------------------------------------------------------------

static
RCDB *init_db (const string & db_file)
{
    RCDB *db;

    db = rc_db_new (db_file);
    if (db == NULL) {
	ERR << "Can't access database '" << db_file << "'" << endl;
	return NULL;
    }
    rc_db_begin (db);

    return db;
}


static void
finish_db (RCDB *db)
{
    rc_db_commit (db);

    rc_db_close (db);
    delete (db);
}


static bool
write_resolvable_to_db (RCDB *db, ResObject::constPtr obj, bool is_installed)
{
    Resolvable::constPtr res = obj;
    Package::constPtr pkg = asKind<Package>(res);

    sqlite_int64 id = write_resolvable (db, obj, pkg, is_installed);
    if (id <= 0) return false;

    write_package (db, id, pkg);
    write_resolvable_deps (db, id, obj);

    return true;
}

//-----------------------------------------------------------------------------

void
write_store_to_db (const string & db_file, const ResStore & store, bool is_target)
{
    RCDB *db = init_db (db_file);
    if (db == NULL) return;
    MIL << "write_store_to_db()" << endl;

    if (store.empty()) {
	ERR << "Couldn't access the packaging system:" << endl;
	return;
    }

    for (ResStore::const_iterator iter = store.begin(); iter != store.end(); ++iter) {
	if (!write_resolvable_to_db (db, *iter, is_target))
	    break;
    }

    finish_db (db);
    return;
}

#if 0
void
write_resolvables_to_db (const std::string & db_file, const ResolvableList & resolvables, bool is_installed)
{
    RCDB *db = init_db (db_file);
    if (db == NULL) return;
    MIL << "write_resolvables_to_db()" << endl;

    for (ResolvableList::const_iterator iter = resolvables.begin(); iter != resolvables.end(); ++iter) {
	if (!write_resolvable_to_db (db, *iter, is_installed))
	    break;
    }

    finish_db (db);
    return;
}
#endif
