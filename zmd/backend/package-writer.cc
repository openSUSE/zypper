/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include "package-writer.h"
#include <sqlite3.h>
#include <iostream>

#include "zypp/base/Exception.h"
#include "zypp/base/Logger.h"
#include "zypp/CapSet.h"
#include "zypp/Rel.h"
#include "zypp/Edition.h"
#include "zypp/Package.h"

#include <cstdlib>
#include <string>

using namespace zypp;
using namespace std;

using std::endl;
using std::string;

//----------------------------------------------------------------------------
typedef enum {
    RC_DEP_TYPE_REQUIRE,
    RC_DEP_TYPE_PROVIDE,
    RC_DEP_TYPE_CONFLICT,
    RC_DEP_TYPE_OBSOLETE
} RCDependencyType;

typedef struct {
    sqlite3 *db;
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

static int
arch2zmd (string str)
{
    return 0;
}


// convert description Text (list<string>) to a single string
static string
desc2str (const Text t)
{
    static string s;
    s.clear();
    for (Text::const_iterator it = t.begin(); it != t.end(); ++it) {
	string line = *it;
	string::size_type authors = line.find ("Authors:");		// strip off 'Authors:'
	if (authors != string::npos) {
	    do {
		--authors;
	    } while (line[authors] == ' ' || line[authors] == '\n');
	    line.resize(authors+1);
	}

	if (it != t.begin()) s += "\n";
	s += line;
    }
    return s;
}
//----------------------------------------------------------------------------

static sqlite3_stmt *
prepare_pkg_insert (sqlite3 *db)
{
    int rc;
    sqlite3_stmt *handle = NULL;

    string query (
	"INSERT INTO packages (name, version, release, epoch, arch,"
	"                      section, file_size, installed_size, channel,"
	"                      pretty_name, summary, description, package_filename,"
	"                      signature_filename, installed, local_package, install_only) "
	"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
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
	"  (pkg_key, dep_type, name, version, release, epoch, arch, relation) "
	"VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

    rc = sqlite3_prepare (db, query.c_str(), -1, &handle, NULL);

    if (rc != SQLITE_OK) {
	ERR << "Can not prepare dependency insertion clause: " << sqlite3_errmsg (db) << endl;
	sqlite3_finalize (handle);
	handle = NULL;
    }

    return handle;
}


static bool
create_tables (RCDB *rcdb)
{
    int rc;

    rc = sqlite3_exec (rcdb->db, "DELETE FROM packages", NULL, NULL, NULL);
    if (rc == SQLITE_ERROR) {
	/* Looks like the table doesn't exist */
	rc = sqlite3_exec (rcdb->db,
	                   "CREATE TABLE packages "
	                   "  (key INTEGER PRIMARY KEY AUTOINCREMENT,"
	                   "   name VARCHAR,"
	                   "   version VARCHAR,"
	                   "   release VARCHAR,"
	                   "   epoch INTEGER,"
	                   "   arch INTEGER,"
	                   "   section INTEGER,"
	                   "   file_size INTEGER,"
	                   "   installed_size INTEGER,"
	                   "   channel VARCHAR,"
	                   "   pretty_name VARCHAR,"
	                   "   summary VARCHAR,"
	                   "   description VARCHAR,"
	                   "   package_filename VARCHAR,"
	                   "   signature_filename VARCHAR,"
	                   "   installed INTEGER,"
	                   "   local_package INTEGER,"
	                   "   install_only INTEGER)",
	                   NULL, NULL, NULL);
    }

    if (rc != SQLITE_OK) {
	ERR << "Can not empty or create packages table: " << sqlite3_errmsg (rcdb->db) << endl;
	return false;
    }

    rc = sqlite3_exec (rcdb->db, "DELETE FROM dependencies", NULL, NULL, NULL);
    if (rc == SQLITE_ERROR) {
	/* Looks like the table doesn't exist */
	rc = sqlite3_exec (rcdb->db,
	                   "CREATE TABLE dependencies "
	                   "  (pkg_key INTEGER NOT NULL,"
	                   "   dep_type INTEGER,"
	                   "   name VARCHAR,"
	                   "   version VARCHAR,"
	                   "   release VARCHAR,"
	                   "   epoch INTEGER,"
	                   "   arch INTEGER,"
	                   "   relation INTEGER)",
	                   NULL, NULL, NULL);
    }

    if (rc != SQLITE_OK) {
	ERR << "Can not empty or create dependencies table: " << sqlite3_errmsg (rcdb->db) << endl;
    }

    return rc == SQLITE_OK;
}


static void
rc_db_close (RCDB *rcdb)
{
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
	sqlite3_bind_text (handle, 3, iter->index().c_str(), -1, SQLITE_TRANSIENT);

	Edition edition = iter->edition();

	if (edition != Edition::noedition) {
	    sqlite3_bind_text (handle, 4, edition.version().c_str(), -1, SQLITE_TRANSIENT);
	    sqlite3_bind_text (handle, 5, edition.release().c_str(), -1, SQLITE_TRANSIENT);
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
write_package_deps (RCDB *rcdb, sqlite_int64 id, ResStore::ResT::Ptr res)
{
    write_deps (rcdb, id, RC_DEP_TYPE_REQUIRE, res->dep (Dep::REQUIRES));
    write_deps (rcdb, id, RC_DEP_TYPE_PROVIDE, res->dep (Dep::PROVIDES));
    write_deps (rcdb, id, RC_DEP_TYPE_CONFLICT, res->dep (Dep::CONFLICTS));
    write_deps (rcdb, id, RC_DEP_TYPE_OBSOLETE, res->dep (Dep::OBSOLETES));
}


static sqlite_int64
write_package (RCDB *rcdb, const Resolvable::constPtr obj)
{
    Package::constPtr pkg = asKind<Package>(obj);
    if (pkg == NULL)
    {
	WAR << "Not a package " << *res << endl;
	return -1;
    }

    int rc;
    sqlite3_stmt *handle = rcdb->insert_pkg_handle;

    sqlite3_bind_text (handle, 1, pkg->name().c_str(), -1, SQLITE_STATIC);
    Edition ed = pkg->edition();
    sqlite3_bind_text (handle, 2, ed.version().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text (handle, 3, ed.release().c_str(), -1, SQLITE_STATIC);
    if (ed.epoch() == Edition::noepoch) {
	sqlite3_bind_int (handle, 4, 0);
    } else {
	sqlite3_bind_int (handle, 4, ed.epoch());
    }

    sqlite3_bind_int (handle, 5, arch2zmd (pkg->arch().asString()));						//pkg->arch().asString().c_str());
    sqlite3_bind_int (handle, 6, 1);						//pkg->section);
    sqlite3_bind_int64 (handle, 7, pkg->archivesize());
    sqlite3_bind_int64 (handle, 8, pkg->size());
    sqlite3_bind_text (handle, 9, "@system", -1, SQLITE_STATIC);		//rc_channel_get_id (pkg->channel)
    sqlite3_bind_text (handle, 10, "", -1, SQLITE_STATIC);			// pkg->pretty_name
    sqlite3_bind_text (handle, 11, pkg->summary().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text (handle, 12, desc2str (pkg->description()).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text (handle, 13, "", -1, SQLITE_STATIC);			//pkg->package_filename
    sqlite3_bind_text (handle, 14, "", -1, SQLITE_STATIC);			//pkg->signature_filename
    sqlite3_bind_int (handle, 15, 1);						//pkg->installed
    sqlite3_bind_int (handle, 16, 0);						//pkg->local_package
    sqlite3_bind_int (handle, 17, 0);						//pkg->install_only

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


void
write_store_to_db (const string & db_file, const ResStore & store)
{
    RCDB *db = init_db (db_file);
    if (db == NULL) return;

    if (store.empty()) {
	ERR << "Couldn't access the packaging system:" << endl;
	return;
    }

    for (ResStore::const_iterator iter = store.begin(); iter != store.end(); ++iter) {
		// *iter == const ResStore::ResT::constPtr res
	sqlite_int64 id = write_package (db, *iter);
	if (id > 0) {
	    write_package_deps (db, id, *iter);
	}
    }

    finish_db (db);
    return;
}


void
write_resolvables_to_db (const std::string & db_file, const ResolvableList & resolvables)
{
    RCDB *db = init_db (db_file);
    if (db == NULL) return;

    for (ResStore::const_iterator iter = resolvables.begin(); iter != resolvables.end(); ++iter) {
		// *iter == const Package::constPtr
	sqlite_int64 id = write_package (db, *iter);
	if (id > 0) {
	    write_package_deps (db, id, *iter);
	}
    }

    finish_db (db);
    return;
}
