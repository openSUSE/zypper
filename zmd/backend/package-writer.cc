/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include "package-writer.h"
#include <sqlite3.h>
#include "zypp/target/rpm/librpmDb.h"
#include "zypp/base/Exception.h"
#include "zypp/base/Logger.h"
#include "zypp/CapSet.h"
#include <cstdlib>
#include <string>

using namespace zypp::target::rpm;

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
	sqlite3_bind_text (handle, 3, iter->index().c_str(), -1, SQLITE_STATIC);
#if 0
	sqlite3_bind_text (handle, 4, spec->version, -1, SQLITE_STATIC);
	sqlite3_bind_text (handle, 5, spec->release, -1, SQLITE_STATIC);
	sqlite3_bind_int (handle, 6, spec->epoch);
	sqlite3_bind_int (handle, 7, spec->arch);
	sqlite3_bind_int (handle, 8, rc_package_dep_get_relation (dep));
#endif
	rc = sqlite3_step (handle);
	sqlite3_reset (handle);

	if (rc != SQLITE_DONE) {
	    ERR << "Error adding package to SQL: " << sqlite3_errmsg (rcdb->db) << endl;
	}
    }
}


static void
write_package_deps (RCDB *rcdb, sqlite_int64 id, RpmHeader::constPtr pkg)
{
    write_deps (rcdb, id, RC_DEP_TYPE_REQUIRE, pkg->tag_requires());
    write_deps (rcdb, id, RC_DEP_TYPE_PROVIDE, pkg->tag_provides());
    write_deps (rcdb, id, RC_DEP_TYPE_CONFLICT, pkg->tag_conflicts());
    write_deps (rcdb, id, RC_DEP_TYPE_OBSOLETE, pkg->tag_obsoletes());
}


static sqlite_int64
write_package (RCDB *rcdb, RpmHeader::constPtr pkg)
{
    int rc;
    sqlite3_stmt *handle = rcdb->insert_pkg_handle;

    sqlite3_bind_text (handle, 1, pkg->tag_name().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text (handle, 2, pkg->tag_version().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text (handle, 3, pkg->tag_release().c_str(), -1, SQLITE_STATIC);
    if (pkg->tag_epoch().empty()) {
	sqlite3_bind_int (handle, 4, 0);
    } else {
	int epoch = atoi (pkg->tag_epoch().c_str());
	sqlite3_bind_int (handle, 4, epoch);
    }

    sqlite3_bind_int (handle, 5, 1);						//pkg->arch().asString().c_str());
    sqlite3_bind_int (handle, 6, 1);						//pkg->section);
    sqlite3_bind_int64 (handle, 7, pkg->tag_archivesize());
    sqlite3_bind_int64 (handle, 8, pkg->tag_size());
    sqlite3_bind_text (handle, 9, "@suse", -1, SQLITE_STATIC);		//rc_channel_get_id (pkg->channel)
    sqlite3_bind_text (handle, 10, "", -1, SQLITE_STATIC);			// pkg->pretty_name
    sqlite3_bind_text (handle, 11, pkg->tag_summary().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text (handle, 12, pkg->tag_description().c_str(), -1, SQLITE_STATIC);
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
fprintf (stderr, "%s-%s-%s : %lld\n", pkg->tag_name().c_str(),pkg->tag_version().c_str(),pkg->tag_release().c_str(), rowid);fflush(stderr);

    return rowid;
}

//-----------------------------------------------------------------------------

void
write_packages_to_db (const string & db_file)
{
    RCDB *db;

    db = rc_db_new (db_file);
    rc_db_begin (db);

    librpmDb::db_const_iterator iter;

    if (iter.dbHdrNum() == 0) {
	ERR << "Couldn't access the packaging system:" << endl;
	return;
    }

    while (*iter) {
	sqlite_int64 id = write_package (db, *iter);
	if (id > 0) {
	    write_package_deps (db, id, *iter);
	}
	++iter;
    }

    rc_db_commit (db);
    rc_db_begin (db);

    while (*iter) {
    }

    rc_db_commit (db);
    rc_db_close (db);
    delete (db);
}
