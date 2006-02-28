/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
// query-files.cc
// zmd helper to extract data from .rpm files
//
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

#include "zypp/target/rpm/RpmHeader.h"
#include "zypp/target/rpm/RpmDb.h"
#include "zypp/Target.h"

using namespace zypp;
using namespace std;

#include <sqlite3.h>
#include <cstring>

#include <sys/stat.h>

#include "dbsource/DbAccess.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "query-files"

//-----------------------------------------------------------------------------

static ResStore
query_file (const Pathname & path)
{
    ResStore store;

    MIL << "query_file(" << path << ")" << endl;

    target::rpm::RpmHeader::constPtr header = target::rpm::RpmHeader::readPackage( path );

    Package::Ptr package = target::rpm::RpmDb::makePackageFromHeader( header, NULL, path );

    if (package != NULL) {
	store.insert( package );
    }

    return store;
}

//
// extract value from "key=value" token string
//

static string
extract_value (const string & token)
{
    char *eq;

    eq = strchr (token.c_str(), '=');

    if (!eq)
	return string();

    while (*(++eq) == ' ');    /* Move past the equals sign and strip leading whitespace */

    return string (eq);
}


// check query for "recursive=N"
//   set recursive = true in N numeric and > 0
//

static void
parse_query (const string & query, bool *recursive)
{
    std::vector<std::string> tokens;
    str::split( query, std::back_inserter( tokens ), ";" );

    for (int pos = 0; pos < tokens.size(); ++pos) {
	string tok = str::toLower( tokens[pos] );
	if (strncmp (tok.c_str(), "recursive", 9) == 0) {
	    string val = extract_value( tok );

	    if (str::strtonum<int>( val ) > 0)
		*recursive = true;
	}

	/* Ignore unknown parts */
    }
}

//-----------------------------------------------------------------------------


int
extract_packages_from_directory (ResStore & store,
				 const Pathname & path,
				 const string & alias,
				 bool recursive)
{
    Pathname filename;
    PathInfo magic;
    bool distro_magic, pkginfo_magic;

DBG << "extract_packages_from_directory(.., " << path << ", " << alias << ", " << recursive << ")" << endl;
    
    /*
      Check for magic files that indicate how to treat the
      directory.  The files aren't read -- it is sufficient that
      they exist.
    */

    magic = PathInfo( path + "/RC_SKIP" );
    if (magic.isExist()) {
        return 0;
    }

    magic = PathInfo( path + "/RC_RECURSIVE" );
    if (magic.isExist())
        recursive = true;
    
    magic = PathInfo( path + "/RC_BY_DISTRO" );
    distro_magic = magic.isExist();

    pkginfo_magic = true;
    magic = PathInfo( path + "/RC_IGNORE_PKGINFO" );
    if (magic.isExist())
        pkginfo_magic = false;

    /* If distro_magic is set, we search for packages in two
       subdirectories of path: path/distro-target (i.e.
       path/redhat-9-i386) and path/x-cross.
    */

#if 0			// commented out it libredcarpet
    if (distro_magic) {
        Pathname distro_path, cross_distro_path;
        bool found_distro_magic = false;
        int count = 0, c;

        distro_path = path + rc_distro_get_target ();
        if (PathInfo(distro_path).isDir()) {
            found_distro_magic = true;

            c = rc_extract_packages_from_directory (distro_path,
                                                    channel, packman,
                                                    callback, user_data);
            if (c >= 0)
                count += c;
        }

        cross_distro_path = g_strconcat (path, "/x-distro", NULL);
        if (g_file_test (cross_distro_path, G_FILE_TEST_IS_DIR)) {
            c = rc_extract_packages_from_directory (cross_distro_path,
                                                    channel, packman,
                                                    callback, user_data);
            if (c >= 0)
                count += c;
        }

        g_free (cross_distro_path);
        g_free (distro_path);

        return count;
    }
#endif

#if 0	// helix format unsupported

    /* If pkginfo_magic is set and if a packageinfo.xml or
       packageinfo.xml.gz file exists in the directory, use it
       instead of just scanning the files in the directory
       looking for packages. */

    if (pkginfo_magic) {
        int i, count;
        gchar *pkginfo_path = NULL;
        const gchar *pkginfo[] = { "packageinfo.xml",
                                   "packageinfo.xml.gz",
                                   NULL };

        for (i = 0; pkginfo[i]; i++) {
            pkginfo_path = g_build_path (G_DIR_SEPARATOR_S, path, pkginfo[i], NULL);
            if (g_file_test (pkginfo_path, G_FILE_TEST_EXISTS))
                break;

            g_free (pkginfo_path);
            pkginfo_path = NULL;
        }

        if (pkginfo_path) {
            PackagesFromDirInfo info;

            info.user_callback = callback;
            info.user_data = user_data;
            info.path = path;

            count = rc_extract_packages_from_helix_file (pkginfo_path,
                                                         channel,
                                                         packages_from_dir_cb,
                                                         &info);
            g_free (pkginfo_path);
            return count;
        }
    }
#endif

    std::list<std::string> dircontent;
    if (filesystem::readdir( dircontent, path, false) != 0) {		// dont look for dot files
	ERR << "readdir " << path << " failed" << endl;
        return -1;
    }

    for (std::list<std::string>::const_iterator it = dircontent.begin(); it != dircontent.end(); ++it) {
	Pathname file_path = path + *it;
	PathInfo file_info( file_path );
        if (recursive && file_info.isDir()) {

	    extract_packages_from_directory( store, file_path, alias, recursive );

        } else if (file_info.isFile()) {

	    string::size_type dotpos = it->find_last_of(".");
	    if (dotpos == string::npos)
		continue;
	    if (string(*it, ++dotpos) != "rpm")
		continue;
	    target::rpm::RpmHeader::constPtr header = target::rpm::RpmHeader::readPackage( file_path );
	    Package::Ptr package = target::rpm::RpmDb::makePackageFromHeader( header, NULL, file_path );

	    if (package != NULL) {
		DBG << "Adding package " << *package << endl;
		store.insert( package );
	    }
        }
    }

}

static ResStore
query_directory (const Pathname & path, bool recursive)
{
    ResStore store;
    MIL << "query_directory( " << path << (recursive?", recursive":"") << ")" << endl;
    extract_packages_from_directory( store, path, "@local", recursive );

    return store;
}

//----------------------------------------------------------------------------

static ResStore
query (const string & uri, const string & channel_id)
{
    ResStore store;

    /* The magic 7 is strlen ("file://") */

    if (uri.size() < 7
	|| uri.compare( 0, 7, "file://") != 0)
    {
	ERR << "Invalid uri '" << uri << "'" << endl;
	return store;
    }

    /* Find the path. The "+ 7" part moves past "file://" */
    string::size_type query_part = uri.find( "?" );

    Pathname path;

    if (query_part != string::npos) {
	string p( uri, 7, query_part - 7 );
	path = p;
    }
    else {
	string p( uri, 7 );
	path = p;
    }

    MIL << "query(" << uri << ") path '" << path << "'" << endl;

#if 0
    channel = rc_channel_new (channel_id != NULL ? channel_id : "@local",
		              "foo", "foo", "foo");
#endif

    struct stat buf;

    int err = stat (path.asString().c_str(), &buf);

    if (err != 0) {
	ERR << "Invalid path '" << path << "'" << endl;
    }
    else if (S_ISREG( buf.st_mode )) {			/* Single file */

	store = query_file( path );

    }
    else if (S_ISDIR( buf.st_mode )) {			/* Directory */

	bool recursive = false;

	if (query_part != string::npos) {
	    /* + 1 to move past the "?" */
	    string p( uri, query_part + 1 );
	    parse_query( p, &recursive );
	}
	store = query_directory( path, recursive );
    }

    return store;
}

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

#define CATALOGSYNC "/installation"

int
main (int argc, char **argv)
{
    if (argc < 3 || argc > 4) {
	std::cerr << "usage: " << argv[0] << " <database> <uri> [catalog id]" << endl;
	return 1;
    }

    const char *logfile = getenv("ZYPP_LOGFILE");
    if (logfile != NULL)
	zypp::base::LogControl::instance().logfile( logfile );
    else
	zypp::base::LogControl::instance().logfile( ZMD_BACKEND_LOG );

    MIL << "START query-files " << argv[1] << " " << argv[2] << " " << ((argc>3)?argv[3]:"") << endl;

    ZYpp::Ptr God = zypp::getZYpp();

    try {
       God->initTarget( "/", true );
    }
    catch( const Exception & excpt_r ) {
       ERR << "Can't initialize target." << endl;
       ZYPP_CAUGHT( excpt_r );
       return 1;
    }

    DbAccess db(argv[1]);

    db.openDb( true );		// open for writing

    if (strcmp( argv[2], CATALOGSYNC ) == 0) {
	MIL << "Doing a catalog sync" << endl;
	sync_catalogs( db );
    }
    else {
	MIL << "Doing a file/directory query" << endl;
	ResStore store = query( argv[2], argc == 4 ? argv[3] : "" );
	if (!store.empty()) {
	    db.writeStore( store, ResStatus::uninstalled );
	}
    }

    db.closeDb();

    MIL << "END query-files" << endl;

    return 0;
}
