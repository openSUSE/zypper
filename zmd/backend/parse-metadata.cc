/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
//
// parse-metadata.cc
//
// parse-metadata <zmd.db> <metadata type> <path> <catalog id>
//
// metadata type can be currently either 'yum' or 'installation'.
// path would be the path on the local file system. Here's an example for
// yum:
// 
// parse-metadata zmd.db yum /var/cache/zmd/foo/ 2332523
// 
// The helper is guaranteed to find /var/cache/zmd/foo/repodata/repomd.xml
// and all the other files referenced from the repomd.xml. For example, if
// the repomd.xml has

#include <iostream>
#include <cstring>
#include <list>

#include "zmd-backend.h"

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/SourceManager.h"
#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

using std::endl;
using namespace zypp;

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parse-metadata"

#include <sqlite3.h>
#include <cstring>

#include <sys/stat.h>

#include "dbsource/DbAccess.h"

//----------------------------------------------------------------------------
// upload all zypp sources as catalogs to the database

static void
sync_source( DbAccess & db, Source_Ref source )
{
    DBG << "sync_source, alias '" << source.alias() << "'" << endl;

    std::string catalog = source.alias();

    if (db.haveCatalog( catalog ) ) {
	db.removeCatalog( catalog );		// clean old entries first
    }

    std::string name = source.zmdName();
    if (name.empty()) name = source.url().asString();
    std::string desc = source.zmdDescription();
    if (desc.empty()) desc = source.vendor();

    if (db.insertCatalog( catalog, name, catalog, desc )) {		// create catalog

	ResStore store = source.resolvables();

	DBG << "Source provides " << store.size() << " resolvables" << endl;

	db.writeStore( store, ResStatus::uninstalled, source.alias().c_str() );	// store all resolvables

    }

    return;
}

static void
sync_catalogs( DbAccess & db )
{
    SourceManager_Ptr manager = SourceManager::sourceManager();

    try {
	manager->restore("/");
    }
    catch (Exception & excpt_r) {
	ZYPP_CAUGHT (excpt_r);
	ERR << "Couldn't restore sources" << endl;
	return;
    }

    std::list<unsigned int> sources = manager->allSources();
    MIL << "Found " << sources.size() << " sources" << endl;

    for (std::list<unsigned int>::const_iterator it = sources.begin(); it != sources.end(); ++it) {
	Source_Ref source = manager->findSource( *it );
	if (!source) {
	    ERR << "SourceManager can't find source " << *it << endl;
	    continue;
	}
	sync_source( db, source );
    }
    return;
}

//----------------------------------------------------------------------------

#define SOURCES "installation"
#define YUM "yum"

int
main (int argc, char **argv)
{
    if (argc != 5) {
	std::cerr << "usage: " << argv[0] << " <database> <type> <path> <catalog id>" << endl;
	return 1;
    }

    const char *logfile = getenv("ZYPP_LOGFILE");
    if (logfile != NULL)
	zypp::base::LogControl::instance().logfile( logfile );
    else
	zypp::base::LogControl::instance().logfile( ZMD_BACKEND_LOG );

    MIL << "START parse-metadata " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << endl;

    ZYpp::Ptr God = zypp::getZYpp();

    DbAccess db(argv[1]);

    db.openDb( true );		// open for writing

    if (strcmp( argv[2], SOURCES ) == 0) {
	MIL << "Doing a catalog sync" << endl;
	sync_catalogs( db );
    }
    else if (strcmp( argv[2], YUM ) == 0) {
	MIL << "Doing a cached source sync" << endl;
	Pathname p;
	Url url( std::string("file:") + argv[3] );
	std::string alias( argv[4] );
	Locale lang( "en" );

	Pathname cache_dir("");
	Source_Ref source( SourceFactory().createFrom(url, p, alias, cache_dir) );

	sync_source ( db, source );
    }
    else {
	ERR << "Invalid option " << argv[2] << ", expecting '" << SOURCES << "' or '" << YUM << "'" << endl;
    }

    db.closeDb();

    MIL << "END parse-metadata" << endl;

    return 0;
}
