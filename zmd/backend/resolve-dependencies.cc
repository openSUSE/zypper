/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

using std::endl;
using namespace zypp;

#include <sqlite3.h>
#include "dbsource/DbAccess.h"
#include "dbsource/DbSources.h"

#include "transactions.h"

int
main (int argc, char **argv)
{
    if (argc != 2) {
	ERR << "usage: " << argv[0] << " <database>" << endl;
	return 1;
    }

    // access the sqlite db

    DbAccess db (argv[1]);
    if (!db.openDb(false))
	return 1;

    // start ZYPP

    ZYppFactory zf;
    ZYpp::Ptr God = zf.getZYpp();

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

    God->resolver()->resolvePool();

    return 0;
}
