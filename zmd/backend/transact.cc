/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include <cstdlib>	// setenv
#include <iostream>
#include <string>
#include <list>

#include "zmd-backend.h"

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include "zypp/ExternalProgram.h"

using namespace std;
using namespace zypp;

#include <sqlite3.h>
#include "dbsource/DbAccess.h"
#include "dbsource/DbSources.h"

#include "transactions.h"
#include "zypp/solver/detail/ResolverInfo.h"

#include "RpmCallbacks.h"

using solver::detail::ResolverInfo_Ptr;

typedef std::list<PoolItem> PoolItemList;

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "transact"

//-----------------------------------------------------------------------------

static void
append_dep_info (ResolverInfo_Ptr info, void *user_data)
{
    string *dep_failure_info = (string *)user_data;
    bool debug = false;

    if (info == NULL) {
	ERR << "append_dep_info(NULL)" << endl;
	return;
    }

    if (getenv ("RCD_DEBUG_DEPS"))
        debug = true;

    if (debug || info->important()) {
	dep_failure_info->append( "\n" );
	if (debug && info->error())
	    dep_failure_info->append( "ERR " );
	if (debug && info->important())
	    dep_failure_info->append( "IMP " );
	dep_failure_info->append( info->message() );
    }
    return;
}


int
main (int argc, char **argv)
{
    if (argc != 2) {
	cerr << "usage: " << argv[0] << " <database>" << endl;
	return 1;
    }

    const char *logfile = getenv("ZYPP_LOGFILE");
    if (logfile != NULL)
	zypp::base::LogControl::instance().logfile( logfile );
    else
	zypp::base::LogControl::instance().logfile( ZMD_BACKEND_LOG );

    MIL << "START transact " << argv[1] << endl;

    // access the sqlite db

    DbAccess db (argv[1]);
    if (!db.openDb(false))
	return 1;

    // start ZYPP

    ZYpp::Ptr God = zypp::getZYpp();

    // load the catalogs and resolvables from sqlite db

    DbSources dbs(db.db());

    const SourcesList & sources = dbs.sources();

    for (SourcesList::const_iterator it = sources.begin(); it != sources.end(); ++it) {
	zypp::ResStore store = it->resolvables();
	MIL << "Catalog " << it->id() << " contributing " << store.size() << " resolvables" << endl;
	God->addResolvables( store, (it->id() == "@system") );
    }

    // now the pool is complete, add transactions

    if (!read_transactions (God->pool(), db.db(), dbs))
	return 1;

    try {
	God->initTarget("/", true);
	PoolItemList x,y,z;

	::setenv( "YAST_IS_RUNNING", "1", 1 );

	RpmCallbacks callbacks;

	God->target()->commit( God->pool(), 0, x, y, z );

	ExternalProgram suseconfig( "/sbin/SuSEconfig", ExternalProgram::Discard_Stderr );	// should redirect stderr to logfile
	suseconfig.close();		// discard exit code
    }
    catch ( Exception & expt_r ) {
	ZYPP_CAUGHT( expt_r );
	return 1;
    }

    db.closeDb();

    MIL << "END transact" << endl;

    return 0;
}
