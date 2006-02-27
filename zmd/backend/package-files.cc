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

#include "zypp/target/rpm/RpmHeader.h"
#include "zypp/target/rpm/RpmDb.h"
#include "zypp/Target.h"

using namespace zypp;
using namespace std;
using target::rpm::RpmHeader;
using target::rpm::FileInfo;

#include <sqlite3.h>
#include <cstring>

#include <sys/stat.h>

#include "dbsource/DbAccess.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "package-files"

//-----------------------------------------------------------------------------

// not needed
#if 0
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
#endif


static int
package_files( sqlite3 *db, sqlite_int64 id, Target_Ptr target )
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

    RpmHeader::constPtr header = NULL;

    if (installed) {

	Edition edition( version, release, epoch );

	try {
	    // get data from rpmdb
	    target->rpmDb().getData( name, edition, header );
	}
	catch( const Exception & excpt_r ) {
	    header = NULL;
	    ERR << "Can't extract header data from rpmdb." << endl;
	    ZYPP_CAUGHT( excpt_r );
	}

#if 0	// not needed
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
#endif

    }
    else {
	try {
	    // get data from package_filename
	    header = RpmHeader::readPackage( package_filename );
	}
	catch( const Exception & excpt_r ) {
	    header = NULL;
	    ERR << "Can't extract header data from '" << package_filename << "'." << endl;
	    ZYPP_CAUGHT( excpt_r );
	}
    }

    if (header == NULL)
	return 1;

MIL << "RpmHeader @" << header << endl;

    sql =
	"INSERT INTO files (resolvable_id, filename, size, md5sum, uid, "
	"                   gid, mode, mtime, ghost, link_target) "
	"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    rc = sqlite3_prepare (db, sql, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not prepare insert-into-files clause: " << sqlite3_errmsg (db) << endl;
	return 1;
    }

    std::list<FileInfo> files = header->tag_fileinfos();

    for (std::list<FileInfo>::const_iterator it = files.begin(); it != files.end(); ++it) {

	sqlite3_bind_int64 (handle, 1, id);
	sqlite3_bind_text  (handle, 2, it->filename.asString().c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int   (handle, 3, it->size);
	sqlite3_bind_text  (handle, 4, it->md5sum.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int   (handle, 5, it->uid);
	sqlite3_bind_int   (handle, 5, it->gid);
	sqlite3_bind_int   (handle, 6, it->mode);
	sqlite3_bind_int   (handle, 7, it->mtime);
	sqlite3_bind_int   (handle, 8, it->ghost);
	sqlite3_bind_text  (handle, 4, it->link_target.asString().c_str(), -1, SQLITE_STATIC);

	rc = sqlite3_step (handle);
	sqlite3_reset (handle);

	if (rc != SQLITE_DONE) {
	    ERR << "Error adding file to SQL: " << sqlite3_errmsg (db) << endl;
	}
    }

    sqlite3_exec (db, "COMMIT", NULL, NULL, NULL);

cleanup:
    sqlite3_finalize (handle);

    return result;
}

//----------------------------------------------------------------------------

int
main (int argc, char **argv)
{
    if (argc != 3) {
	std::cerr << "usage: " << argv[0] << " <database> <package id>" << endl;
	return 1;
    }

    const char *logfile = getenv("ZYPP_LOGFILE");
    if (logfile != NULL)
	zypp::base::LogControl::instance().logfile( logfile );
    else
	zypp::base::LogControl::instance().logfile( ZMD_BACKEND_LOG );

    MIL << "START package-files " << argv[1] << " " << argv[2] << endl;

    ZYpp::Ptr God = zypp::getZYpp();

    DbAccess db(argv[1]);

    db.openDb( true );		// open for writing
    Target_Ptr target;

    try {
	God->initTarget( "/", true );
	target = God->target();
    }
    catch( const Exception & excpt_r ) {    
	ERR << "Can't initialize target." << endl;
	ZYPP_CAUGHT( excpt_r );
	return 1;
    }

    int result = package_files( db.db(), str::strtonum<long long>( argv[2] ), target );

    God->finishTarget();

    db.closeDb();

    MIL << "END package-files" << endl;

    return result;
}
