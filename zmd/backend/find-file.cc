/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

using std::endl;
using namespace zypp;

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "find-file"

#include <sqlite3.h>
#include "resolvable-writer.h"

static gboolean
find_package_cb (RCPackage *package, gpointer user_data)
{
    g_print ("%d\n", package->id);
    return TRUE;
}

static void
print_package_ids (ResolvableList *packages)
{
    RCPackage *package;
    RCPackageDep *dep;
    RCPackageMatch *match;
    RCWorld *world = rc_get_world ();

    for (iter = packages; iter; iter = iter->next) {
        package = iter->data;

        dep = rc_package_dep_new_from_spec (RC_PACKAGE_SPEC (package),
                                            RC_RELATION_EQUAL,
                                            RC_CHANNEL_SYSTEM,
                                            FALSE, FALSE);
        match = rc_package_match_new ();
        rc_package_match_set_dep (match, dep);

        rc_world_foreach_package_by_match (world, match,
                                           find_package_cb, NULL);

        rc_package_match_free (match);
        rc_package_dep_unref (dep);
    }
}

int
main (int argc, char **argv)
{
    if (argc != 3) {
	ERR << "usage: " << argv[0] <<  " <database> <filename>" << endl;
        return 1;
    }

    MIL << "START find-file " << argv[1] << " " << argv[2] << endl;

    ZYppFactory zf;
    ZYpp::Ptr God = zf.getZYpp();

    try {
	God->initTarget("/");
    }
    catch (Exception & excpt_r) {
	ZYPP_CAUGHT (excpt_r);
	return 1;
    }

    
    packages = rc_packman_find_file (packman, argv[2]);
    if (packages == NULL) {
        if (rc_packman_get_error (packman))
            rc_debug (RC_DEBUG_LEVEL_ERROR, rc_packman_get_reason (packman));
        ret = 1;
        goto cleanup;
    }

    world = (RCWorld *) rc_world_sql_new (argv[1]);
    if (!world) {
        ret = 1;
        goto cleanup;
    }

    rc_world_multi_add_subworld (RC_WORLD_MULTI (rc_get_world ()), world);
    g_object_unref (world);

    print_package_ids (packages);

 cleanup:
    rc_package_slist_unref (packages);
    g_slist_free (packages);

    rc_set_world (NULL);
    g_object_unref (packman);

    MIL << "END find-file" << endl;

    return ret;
}
