#include <iostream>

#include "zypp/base/Logger.h"
#include "zmd/backend/dbsource/DbSources.h"

using std::endl;

int
main(int argc, char *argv[])
{
    if (argc != 2) {
	ERR << "usage: " << argv[0] << " <database>" << endl;
	return 1;
    }

    sqlite3 *db = NULL;
    int rc;

    rc = sqlite3_open (argv[1], &db);
    if (rc != SQLITE_OK) {
	ERR << "Can not open SQL database '" << argv[1] << "': " << sqlite3_errmsg (db) << endl;
	return 1;
    }
    MIL << "Database openend" << endl;

    sqlite3_exec (db, "PRAGMA synchronous = 0", NULL, NULL, NULL);
    sqlite3_exec (db, "BEGIN", NULL, NULL, NULL);

    MIL << "Calling dbsources" << endl;

    zypp::DbSources s(db);

    s.sources();

    MIL << "Closing down" << endl;

    sqlite3_exec (db, "COMMIT", NULL, NULL, NULL);

    sqlite3_close (db);

    return 0;
}
