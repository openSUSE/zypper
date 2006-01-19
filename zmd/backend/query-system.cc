/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include <libredcarpet.h>
#include <sqlite3.h>
#include "package-writer.h"

static void
set_channel (gpointer data, gpointer user_data)
{
    RCPackage *package = data;
    RCChannel *channel = user_data;

    package->channel = rc_channel_ref (channel);
}

static void
debug (const char *message, RCDebugLevel level, gpointer user_data)
{
    fprintf (stderr, "%d|%s\n", level, message);
}

int
main (int argc, char **argv)
{
    RCPackman *packman;
    RCPackageSList *packages = NULL;

    rc_debug_add_handler (debug, RC_DEBUG_LEVEL_ALWAYS, NULL);

    if (argc != 2) {
        rc_debug (RC_DEBUG_LEVEL_ERROR,
                  "usage: %s <database>", argv[0]);
        return 1;
    }

    g_type_init ();

    packman = rc_distman_new ();

    if (!packman) {
        rc_debug (RC_DEBUG_LEVEL_ERROR,
                  "Couldn't access the packaging system");
        return 1;
    }

    if (rc_packman_get_error (packman)) {
        rc_debug (RC_DEBUG_LEVEL_ERROR,
                  "Couldn't access the packaging system: %s",
                  rc_packman_get_reason (packman));
        return 1;
    }

    packages = rc_packman_query_all (packman);

    if (packages) {
        RCChannel *channel;

        channel = rc_channel_new ("@system", "foo", "foo", "foo");
        g_slist_foreach (packages, set_channel, channel);
        rc_channel_unref (channel);

        write_packages_to_db (argv[1], packages);
        rc_package_slist_unref (packages);
        g_slist_free (packages);
    }

    g_object_unref (packman);

    return 0;
}
