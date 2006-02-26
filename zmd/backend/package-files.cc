/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
// package-files.cc
// ZMD backend helper

#include <iostream>
#include <cstring>
#include <list>

#include "zmd-backend.h"

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/SourceManager.h"
#include "zypp/Source.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"

#include "zypp/Target.h"
#include "zypp/target/rpm/RpmHeader.h"

using namespace zypp;
using namespace std;
using target::rpm::RpmHeader;

#include <sqlite3.h>
#include <cstring>

#include <sys/stat.h>

#include "dbsource/DbAccess.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "package-files"

//-----------------------------------------------------------------------------

class LookFor : public resfilter::PoolItemFilterFunctor
{
  public:
    PoolItem_Ref item;

    bool operator()( PoolItem_Ref provider )
    {
        item = provider;
        return false;                           // stop here, we found it
    }
};

static int
package_files( sqlite3 *db, long id, const ResPool & pool )
{
    int result = 0;

    // prepare SQL query
MIL << "package_files id " << id << endl;
    sqlite3_stmt *handle = NULL;
    const char *sql =
        //      0     1        2        3      4
	"SELECT name, version, release, epoch, arch, "
        //      5          6
	"       installed, package_filename "
	"FROM packages "
	"WHERE id = ?";

    int rc = sqlite3_prepare (db, sql, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not prepare get-package-by-id clause: " << sqlite3_errmsg (db) << endl;
	return 1;
    }
    rc = sqlite3_bind_int64 (handle, 1, id);

    // execute the query

    rc = sqlite3_step (handle);

    if (rc != SQLITE_ROW) {
	if (rc == SQLITE_DONE)
	    WAR << "Can not read package: package not found" << endl;
	else
	    ERR << "Can not read package: %s" << sqlite3_errmsg (db) << endl;
	sqlite3_finalize (handle);
	return 1;
    }

    // extract query results

    string name;
    const char *cptr = (const char *) sqlite3_column_text (handle, 0);
    if (cptr) name = cptr;
    string version;
    cptr = (const char *) sqlite3_column_text (handle, 1);
    if (cptr) version = cptr;
    string release;
    cptr = (const char *) sqlite3_column_text (handle, 2);
    if (cptr) release = cptr;
    int epoch = sqlite3_column_int (handle, 3);
    Arch arch( DbAccess::Rc2Arch( (RCArch) sqlite3_column_int (handle, 4) ) );
    int installed = sqlite3_column_int (handle, 5);
    Pathname package_filename;
    cptr = (const char *) sqlite3_column_text (handle, 6);
    if (cptr) package_filename = cptr;
MIL << "=> " << name << "-" << version << "-" << release << "." << arch << ": " << ((installed==0)?"not ":"") << "installed" << ", file '" << package_filename.asString() << "'" << endl;

    sqlite3_finalize (handle);

    // get package data

    if (installed) {

	Edition edition( version, release, epoch );
	LookFor info;

	// find package in pool
	invokeOnEach( pool.byNameBegin( name ), pool.byNameEnd( name ),
		      functor::chain (
			functor::chain (resfilter::ByInstalled (),
					resfilter::ByKind( ResTraits<zypp::Package>::kind ) ),
				        resfilter::byEdition<CompareByEQ<Edition> >( edition )),
		      functor::functorRef<bool,PoolItem> (info) );

	if (!info.item) {
	    ERR << "Installed package" << name << "-" << edition << " NOT found" <<endl;
	    return 1;
	}

	MIL << "Id " << id << ": " << info.item << endl;
    }
    else {
	// get data from package_filename
	RpmHeader::constPtr header = RpmHeader::readPackage( package_filename );

    }


    sql =
        "INSERT INTO files (resolvable_id, filename, size, md5sum, uid, "
        "                   gid, mode, mtime, ghost, link_target) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    rc = sqlite3_prepare (db, sql, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not prepare insert-into-files clause: " << sqlite3_errmsg (db) << endl;
	return 1;
    }

cleanup:
    sqlite3_finalize (handle);

    return result;
}

//----------------------------------------------------------------------------

int
main (int argc, char **argv)
{
    const char *logfile = getenv("ZYPP_LOGFILE");
    if (logfile != NULL)
	zypp::base::LogControl::instance().logfile( logfile );
    else
	zypp::base::LogControl::instance().logfile( ZMD_BACKEND_LOG );

    ZYpp::Ptr God = zypp::getZYpp();

    if (argc != 3) {
	std::cerr << "usage: " << argv[0] << " <database> <package id>" << endl;
	return 1;
    }

    DbAccess db(argv[1]);

    db.openDb( true );		// open for writing

    God->initTarget( "/" );

    int result = package_files( db.db(), str::strtonum<long>( argv[2] ), God->pool() );

    God->finishTarget();

    db.closeDb();

    return result;
}
