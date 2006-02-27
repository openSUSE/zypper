/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <iostream>

#include "zmd-backend.h"

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/base/Exception.h"
#include "zypp/target/rpm/RpmDb.h"
#include "zypp/Target.h"

#include "dbsource/DbAccess.h"

using namespace std;
using namespace zypp;

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "find-file"

#include <sqlite3.h>

int
main (int argc, char **argv)
{
    if (argc != 3) {
	ERR << "usage: " << argv[0] <<  " <database> <filename>" << endl;
        return 1;
    }

    const char *logfile = getenv("ZYPP_LOGFILE");
    if (logfile != NULL)
	zypp::base::LogControl::instance().logfile( logfile );
    else
	zypp::base::LogControl::instance().logfile( ZMD_BACKEND_LOG );

    MIL << "START find-file " << argv[1] << " " << argv[2] << endl;

    ZYpp::Ptr God = zypp::getZYpp();
    Target_Ptr target;

    try {
	God->initTarget( "/", true );
	target = God->target();
    }
    catch (Exception & excpt_r) {
	ZYPP_CAUGHT (excpt_r);
	return 1;
    }

    string name = target->rpmDb().whoOwnsFile( argv[2] );
    if (name.empty()) {
	ERR << "File " << argv[2] << " does not belong to any (installed) package" << endl;
	return 1;
    }

    DbAccess dbacc( argv[1] );
    dbacc.openDb( false );
    sqlite3 *db = dbacc.db();

    // prepare SQL query

    sqlite3_stmt *handle = NULL;
    const char *sql =
	//      0
	"SELECT resolvable_id "
	"FROM packages "
	"WHERE name = ? AND installed = 1";

    int rc = sqlite3_prepare (db, sql, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not prepare get-package-by-name clause: " << sqlite3_errmsg (db) << endl;
	return 1;
    }
    rc = sqlite3_bind_text( handle, 1, name.c_str(), -1, SQLITE_STATIC );

    // execute the query

    sqlite_int64 id = -1;

    while ((rc = sqlite3_step (handle)) == SQLITE_ROW) {
	id = sqlite3_column_int64 (handle, 0);
	cout << id << endl;
    }

    int res = 0;

    if (rc != SQLITE_DONE) {
	if (id == -1)
	    WAR << "Can not read package: package '" << name << "' not found" << endl;
	else
	    ERR << "Can not read package: %s" << sqlite3_errmsg (db) << endl;
	res = 1;
    }

    sqlite3_finalize (handle);

    dbacc.closeDb();

    MIL << "END find-file" << endl;

    return res;
}
