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
query_file (Target_Ptr target, const Pathname & path)
{
    ResStore store;

    target::rpm::RpmHeader::constPtr header = target::rpm::RpmHeader::readPackage( path );

    Package::Ptr package = target::rpm::RpmDb::makePackageFromHeader( header );

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

#if 0
typedef struct {
    RCResolvableFn user_callback;
    gpointer    user_data;
    const gchar *path;
} PackagesFromDirInfo;

static gboolean
packages_from_dir_cb (RCPackage *package, gpointer user_data)
{
    PackagesFromDirInfo *info = user_data;
    RCPackageUpdate *update;

    /* Set package path */
    update = rc_package_get_latest_update (package);
    if (update && update->package_url)
        package->package_filename = g_build_path (G_DIR_SEPARATOR_S,
                                                  info->path,
                                                  update->package_url,
                                                  NULL);
    if (info->user_callback)
        return info->user_callback ((RCResolvable *)package, info->user_data);

    return TRUE;
}

gint
rc_extract_packages_from_directory (const char *path,
                                    RCChannel *channel,
                                    RCPackman *packman,
                                    gboolean recursive,
                                    RCResolvableFn callback,
                                    gpointer user_data)
{
    GDir *dir;
    GHashTable *hash;
    struct HashIterInfo info;
    const char *filename;
    char *magic;
    gboolean distro_magic, pkginfo_magic;
    
    g_return_val_if_fail (path && *path, -1);
    g_return_val_if_fail (channel != NULL, -1);

    /*
      Check for magic files that indicate how to treat the
      directory.  The files aren't read -- it is sufficient that
      they exist.
    */

    magic = g_strconcat (path, "/RC_SKIP", NULL);
    if (g_file_test (magic, G_FILE_TEST_EXISTS)) {
        g_free (magic);
        return 0;
    }
    g_free (magic);

    magic = g_strconcat (path, "/RC_RECURSIVE", NULL);
    if (g_file_test (magic, G_FILE_TEST_EXISTS))
        recursive = TRUE;
    g_free (magic);
    
    magic = g_strconcat (path, "/RC_BY_DISTRO", NULL);
    distro_magic = g_file_test (magic, G_FILE_TEST_EXISTS);
    g_free (magic);

    pkginfo_magic = TRUE;
    magic = g_strconcat (path, "/RC_IGNORE_PKGINFO", NULL);
    if (g_file_test (magic, G_FILE_TEST_EXISTS))
        pkginfo_magic = FALSE;
    g_free (magic);

    /* If distro_magic is set, we search for packages in two
       subdirectories of path: path/distro-target (i.e.
       path/redhat-9-i386) and path/x-cross.
    */

#if 0      
    if (distro_magic) {
        char *distro_path, *cross_distro_path;
        gboolean found_distro_magic = FALSE;
        int count = 0, c;

        distro_path = g_strconcat (path, "/", rc_distro_get_target (), NULL);
        if (g_file_test (distro_path, G_FILE_TEST_IS_DIR)) {
            found_distro_magic = TRUE;

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

    dir = g_dir_open (path, 0, NULL);
    if (dir == NULL)
        return -1;

    hash = g_hash_table_new (NULL, NULL);

    while ( (filename = g_dir_read_name (dir)) ) {
        gchar *file_path;

        file_path = g_strconcat (path, "/", filename, NULL);

        if (recursive && g_file_test (file_path, G_FILE_TEST_IS_DIR)) {
            rc_extract_packages_from_directory (file_path,
                                                channel,
                                                packman,
                                                TRUE,
                                                hash_recurse_cb,
                                                hash);
        } else if (g_file_test (file_path, G_FILE_TEST_IS_REGULAR)) {
            RCPackage *pkg;

            pkg = rc_packman_query_file (packman, file_path, TRUE);
            if (pkg != NULL) {
                rc_resolvable_set_channel (RC_RESOLVABLE (pkg), channel);
                pkg->package_filename = g_strdup (file_path);
                pkg->local_package = FALSE;
                add_fake_history (pkg);
                package_into_hash (pkg, hash);
                g_object_unref (pkg);
            }
        }

        g_free (file_path);
    }

    g_dir_close (dir);
   
    info.callback = callback;
    info.user_data = user_data;
    info.count = 0;

    /* Walk across the hash and:
       1) Invoke the callback on each package
       2) Unref each package
    */
    g_hash_table_foreach (hash, hash_iter_cb, &info);

    g_hash_table_destroy (hash);

    return info.count;
}
#endif

static ResStore
query_directory (Target_Ptr target, const Pathname & path, bool recursive)
{
    ResStore store;
#warning Unclear semantics
#if 0
    rc_extract_packages_from_directory (path, channel,
		                        rc_packman_get_global (),
		                        recursive,
		                        prepend_package,
		                        &packages);
#endif
    return store;
}

//----------------------------------------------------------------------------

static ResStore
query (Target_Ptr target, const string & uri, const string & channel_id)
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

	store = query_file (target, path);

    }
    else if (S_ISDIR( buf.st_mode )) {			/* Directory */

	bool recursive = false;

	if (query_part != string::npos) {
	    /* + 1 to move past the "?" */
	    string p( uri, query_part + 1 );
	    parse_query( p, &recursive );
	}
	store = query_directory (target, path, recursive);
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
    const char *logfile = getenv("ZYPP_LOGFILE");
    if (logfile != NULL)
	zypp::base::LogControl::instance().logfile( logfile );
    else
	zypp::base::LogControl::instance().logfile( ZMD_BACKEND_LOG );

    ZYpp::Ptr God = zypp::getZYpp();

    try {
	God->initTarget( "/" );
//	target = God->target();
    }
    catch( const Exception & excpt_r ) {    
	ERR << "Can't initialize target." << endl;
	ZYPP_CAUGHT( excpt_r );
	return 1;
    }

    if (argc < 3 || argc > 4) {
	std::cerr << "usage: " << argv[0] << " <database> <uri> [catalog id]" << endl;
	return 1;
    }

    DbAccess db(argv[1]);

    db.openDb( true );		// open for writing

    if (strcmp( argv[2], CATALOGSYNC ) == 0) {
	MIL << "Doing a catalog sync" << endl;
	sync_catalogs( db );
    }
    else {
	MIL << "Doing a file query" << endl;
	ResStore store = query (God->target(), argv[2], argc == 4 ? argv[3] : "");
	if (!store.empty()) {
	    db.writeStore( store, ResStatus::uninstalled );
	}
    }

    db.closeDb();

    return 0;
}
