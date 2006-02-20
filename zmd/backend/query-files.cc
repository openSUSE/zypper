/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

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

#include "zypp/Target.h"

using std::endl;
using namespace zypp;

#include <sqlite3.h>
#include <cstring>

#include <sys/stat.h>

#include "dbsource/DbAccess.h"

//-----------------------------------------------------------------------------

static ResStore
query_file (Target_Ptr target, const char *path)
{
    ResStore store;
#if 0
    Resolvable::constPtr resolvable = target->rpm-qp (path);	// FIXME, needs rpm -qp
    if (resolvable != NULL) {
	store.insert( resolvable );
    }
#endif
    return store;
}

static char *
extract_value (char *token)
{
    char *eq, *value;

    eq = strchr (token, '=');

    if (!eq)
	return NULL;

    while (*(++eq) == ' ');    /* Move past the equals sign and strip leading whitespace */

    return strdup (value);
}

static void
parse_query (const char *query, bool *recursive)
{
#if 0
    char **tokens, **t;

    tokens = g_strsplit (query, ";", 0);

    for (t = tokens; t && *t; t++) {
	if (g_strncasecmp (*t, "recursive", 9) == 0) {
	    char *tmp = extract_value (*t);

	    if (atoi (tmp))
		*recursive = TRUE;

	    free (tmp);
	}

	/* Ignore unknown parts */
    }

    g_strfreev (tokens);
#endif
}

static ResStore
query_directory (Target_Ptr target, const char *path, bool recursive)
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
query (Target_Ptr target, const char *uri, const char *channel_id)
{
    char *query_part;
    char *path = NULL;
    ResStore store;

    /* The magic 7 is strlen ("file://") */

    if (strncasecmp (uri, "file://", 7) != 0) {
	ERR << "Invalid uri '" << uri << "'" << endl;
	return store;
    }

    /* Find the path. The "+ 7" part moves past "file://" */
    query_part = strchr (uri + 7, '?');

    if (query_part)
	path = strndup (uri + 7, query_part - uri - 7);
    else
	path = strdup (uri + 7);

#if 0
    channel = rc_channel_new (channel_id != NULL ? channel_id : "@local",
		              "foo", "foo", "foo");
#endif

    struct stat buf;

    int err = stat (path, &buf);

    if (err != 0) {
	ERR << "Invalid path '" << path << "'" << endl;
    }
    else if (S_ISREG(buf.st_mode)) {
	/* Single file */
	store = query_file (target, path);
    } else if (S_ISDIR(buf.st_mode)) {
	/* Directory */
	bool recursive = false;

	if (query_part)
	    /* + 1 to move past the "?" */
	    parse_query (query_part + 1, &recursive);

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

    return;
}

static void
sync_catalogs( DbAccess & db )
{
    SourceManager_Ptr manager = SourceManager::sourceManager();

    std::list<unsigned int> sources = manager->allSources();

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
    zypp::base::LogControl::instance().logfile( ZMD_BACKEND_LOG );

    ZYpp::Ptr God = zypp::getZYpp();

    try {
	God->initTarget("/");
    }
    catch (Exception & excpt_r) {
	ZYPP_CAUGHT (excpt_r);
	ERR << "Couldn't access the packaging system" << endl;
	return 1;
    }

    if (argc < 3 || argc > 4) {
	std::cerr << "usage: " << argv[0] << " <database> <uri> [catalog id]" << endl;
	return 1;
    }

    DbAccess db(argv[1]);

    if (strcmp( argv[2], CATALOGSYNC ) == 0) {
	sync_catalogs( db );
    }
    else {
	ResStore store = query (God->target(), argv[2], argc == 4 ? argv[3] : NULL);
	if (!store.empty()) {
	    db.writeStore( store, ResStatus::uninstalled );
	}
    }

    return 0;
}
