/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include <iostream>

#include "zmd-backend.h"

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

using std::endl;
using namespace zypp;

#include <sqlite3.h>
#include "dbsource/DbAccess.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "query-system"

int
main (int argc, char **argv)
{
    if (argc != 2) {
	std::cerr << "usage: " << argv[0] << " <database>" << endl;
	return 1;
    }

    const char *logfile = getenv("ZYPP_LOGFILE");
    if (logfile != NULL)
	zypp::base::LogControl::instance().logfile( logfile );
    else
	zypp::base::LogControl::instance().logfile( ZMD_BACKEND_LOG );

    MIL << "START query-system " << argv[1] << endl;

    ZYpp::Ptr God = zypp::getZYpp();

    try {
	God->initTarget("/");
    }
    catch (Exception & excpt_r) {
	ZYPP_CAUGHT (excpt_r);
	return 1;
    }

    DbAccess db( argv[1] );
    db.openDb( true );

    db.writeStore( God->target()->resolvables(), ResStatus::installed, "@system" );

    db.closeDb();

    MIL << "END query-system" << endl;

    return 0;
}
