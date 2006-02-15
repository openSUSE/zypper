/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include <list>

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include "zypp/Target.h"

using std::endl;
using namespace zypp;

#include <sqlite3.h>
#include <cstring>

#include <sys/stat.h>

#include "dbsource/DbAccess.h"

static ResObjectList
query_file (Target_Ptr target, const char *path)
{
    ResObjectList resolvables;
#if 0
    Resolvable::constPtr resolvable = target->rpm-qp (path);	// FIXME, needs rpm -qp
    if (resolvable != NULL) {
	resolvables.push_back (resolvable);
    }
#endif
    return resolvables;
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

static ResObjectList
query_directory (Target_Ptr target, const char *path, bool recursive)
{
    ResObjectList resolvables;
#warning Unclear semantics
#if 0
    rc_extract_packages_from_directory (path, channel,
		                        rc_packman_get_global (),
		                        recursive,
		                        prepend_package,
		                        &packages);
#endif
    return resolvables;
}

//----------------------------------------------------------------------------

static ResObjectList
query (Target_Ptr target, const char *uri, const char *channel_id)
{
    char *query_part;
    char *path = NULL;
    ResObjectList resolvables;

    /* The magic 7 is strlen ("file://") */

    if (strncasecmp (uri, "file://", 7) != 0) {
	ERR << "Invalid uri '" << uri << "'" << endl;
	return resolvables;
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
	resolvables = query_file (target, path);
    } else if (S_ISDIR(buf.st_mode)) {
	/* Directory */
	bool recursive = false;

	if (query_part)
	    /* + 1 to move past the "?" */
	    parse_query (query_part + 1, &recursive);

	resolvables = query_directory (target, path, recursive);
    }

    return resolvables;
}

//----------------------------------------------------------------------------

int
main (int argc, char **argv)
{
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
	ERR << "usage: " << argv[0] << " <database> <uri> [catalog id]" << endl;
	return 1;
    }

    DbAccess db(argv[1]);

    ResObjectList resolvables = query (God->target(), argv[2], argc == 4 ? argv[3] : NULL);
    if (!resolvables.empty()) {
	db.writeResObjects( resolvables, true);
    }

    return 0;
}
