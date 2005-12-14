/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * deptestomatic.cc
 *
 * Copyright (C) 2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 */

/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <zypp/solver/detail/libzypp_solver.h>

#include <y2util/stringutil.h>

int assertOutput( const char* output)
{
    printf( "Assertion in %s, %d : %s  --> exit\n",  __FILE__, __LINE__,
             output );
    exit (0);
}

# define assertExit(expr) \
  (__ASSERT_VOID_CAST ((expr) ? 0 :		 \
		       (assertOutput (__STRING(expr)))))



using namespace std;
using namespace ZYPP;

static MultiWorldPtr world = NULL;

typedef list<unsigned int> ChecksumList;

//---------------------------------------------------------------------------

#if 0
static void
lock_resItem (ResItemPtr resItem)
{
    RCResItemDep *dep;
    RCResItemMatch *match;

    dep = rc_resItem_dep_new_from_spec (RC_RESOLVABLE_SPEC (resItem),
					RC_RELATION_EQUAL, RC_TYPE_RESOLVABLE,
					RC_CHANNEL_ANY, false, false);

    match = rc_resItem_match_new ();
    rc_resItem_match_set_dep (match, dep);

    rc_resItem_dep_unref (dep);

    rc_world_add_lock (rc_get_world (), match);
}


static bool
remove_resItem_cb (RCWorld *world, gpointer user_data)
{
    ResItemPtr resItem = user_data;

    rc_world_store_remove_resItem (RC_WORLD_STORE (world), resItem);

    return true;
}


static void
remove_resItem (ResItemPtr resItem)
{
    rc_world_multi_foreach_subworld_by_type (RC_WORLD_MULTI (world),
					     RC_TYPE_WORLD_STORE,
					     remove_resItem_cb, resItem);
}

/* ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** */

#endif	// 0


//==============================================================================================================================

//---------------------------------------------------------------------------------------------------------------------
// helper functions

typedef list<string> StringList;

static void
assemble_install_cb (constResItemPtr resItem,
		     ResItemStatus status,
		     void *data)
{
    StringList *slist = (StringList *)data;
    string str = stringutil::form ("%-7s ",  resItem->isInstalled() ? "|flag" : "install");
    str += resItem->asString();

    slist->push_back (str);
}


static void
assemble_uninstall_cb (constResItemPtr resItem,
		       ResItemStatus status,
		       void *data)
{
    StringList *slist = (StringList *)data;
    string str = stringutil::form ("%-7s ",  resItem->isInstalled() ? "remove" : "|unflag");
    str += resItem->asString();

    slist->push_back (str);
}


static void
assemble_upgrade_cb (constResItemPtr res1,
		     ResItemStatus status1,
		     constResItemPtr res2,
		     ResItemStatus status2,
		     void *data)
{
    StringList *slist = (StringList *)data;
    string str = "upgrade ";

    str += res2->asString();
    str += " => ";
    str += res1->asString();

    slist->push_back (str);
}



static void
print_sep (void)
{
    printf ("\n------------------------------------------------\n\n");
}


static void
print_important (const string & str)
{
    printf (">!> %s\n", str.c_str());
}


static void
print_solution (ResolverContextPtr context, int *count, ChecksumList & checksum_list)
{
    if (context->isValid ()) {

	StringList items;
	items.clear();

	unsigned int checksum = 0;
	bool is_dup = false;

	printf (">!> Solution #%d:\n", *count);
	++*count;

	context->foreachInstall (assemble_install_cb, &items);

	context->foreachUninstall (assemble_uninstall_cb, &items);

	context->foreachUpgrade (assemble_upgrade_cb, &items);

	items.sort ();

	for (StringList::const_iterator iter = items.begin(); iter != items.end(); iter++) {
	    const char *c = (*iter).c_str();
	    while (*c) {
		checksum = 17 * checksum + (unsigned int)*c;
		++c;
	    }
	}
	printf ("Checksum = %x\n", checksum);

	for (ChecksumList::const_iterator iter = checksum_list.begin(); iter != checksum_list.end() && !is_dup; iter++) {
	    if (*iter == checksum) {
		is_dup = true;
	    }
	}

	if (! is_dup) {
	    for (StringList::const_iterator iter = items.begin(); iter != items.end(); iter++) {
		print_important (*iter);
	    }
	    checksum_list.push_back (checksum);
	} else {
	    printf (">!> This solution is a duplicate.\n");
	}

	items.clear();

    } else {
	printf (">!> Failed Attempt:\n");
    }

    printf (">!> installs=%d, upgrades=%d, uninstalls=%d\n", context->installCount(), context->upgradeCount(), context->uninstallCount());
    printf ("download size=%.1fk, install size=%.1fk\n", context->downloadSize() / 1024.0, context->installSize() / 1024.0);
    printf ("total priority=%d, min priority=%d, max priority=%d\n", context->totalPriority(), context->minPriority(), context->maxPriority());
    printf ("other penalties=%d\n",  context->otherPenalties());
    printf ("- - - - - - - - - -\n");
    fflush (stdout);

    context->spewInfo ();
    fflush (stdout);

}


//---------------------------------------------------------------------------------------------------------------------
static bool
mark_as_system_cb (constResItemPtr resItem, void *unused)
{
    ResItemPtr r = ResItemPtr::cast_away_const(resItem);
    r->setInstalled (true);

    return true;
}

static void
undump (const char *filename)
{
    UndumpWorldPtr undump_world;

    undump_world = new UndumpWorld (filename);
    if (undump_world == NULL) {
	fprintf (stderr, "Couldn't undump from file '%s'", filename);
	return;
    }

    world->addSubworld (undump_world);
}


static ChannelPtr
get_channel (const char *channel_name)
{
    ChannelPtr channel;

    channel = world->getChannelById (channel_name);

    if (channel == NULL)
	channel = world->getChannelByAlias (channel_name);

    if (channel == NULL)
	channel = world->getChannelByName (channel_name);

    return channel;
}


static constResItemPtr
get_resItem (const char *channel_name, const char *package_name)
{
    constChannelPtr channel;
    constResItemPtr resItem;

    channel = get_channel (channel_name);

    if (channel == NULL) {
	fprintf (stderr, "Can't find package '%s': channel '%s' not defined\n", package_name, channel_name);
	return NULL;
    }

    resItem = world->findResItem (channel, package_name);

    if (resItem == NULL) {
	fprintf (stderr, "Can't find package '%s' in channel '%s': no such package\n", package_name, channel_name);
	return NULL;
    }

    return resItem;
}

//---------------------------------------------------------------------------------------------------------------------
// setup related functions

static bool
add_to_world_cb (constResItemPtr resItem, void *data)
{
    WorldPtr world = *((WorldPtr *)data);
    ((StoreWorldPtr)world)->addResItem (resItem);

    return true;
}


static void
load_channel (const string & name, const string & filename, const string & type, bool system_packages)
{
    string pathname = "deptestomatic/" + filename;
    
    if (getenv ("RC_SPEW")) fprintf (stderr, "load_channel(%s,%s,%s,%s)\n", name.c_str(), pathname.c_str(), type.c_str(), system_packages?"system":"non-system");

    ChannelType chan_type = system_packages ? CHANNEL_TYPE_SYSTEM : CHANNEL_TYPE_UNKNOWN;
    ChannelPtr channel;
    unsigned int count;
    StoreWorldPtr store = new StoreWorld();

    World::globalWorld()->addSubworld (store);

    if (type == "helix")
	chan_type = CHANNEL_TYPE_HELIX;
    else if (type == "debian")
	chan_type = CHANNEL_TYPE_DEBIAN;
#if 0
    else
	printf ("Unknown channel type '%s', defaulting to helix\n", type.c_str());
#endif
    if (chan_type == CHANNEL_TYPE_UNKNOWN) { /* default to helix */
	chan_type = CHANNEL_TYPE_HELIX;
    }

    channel = new Channel (name, name, name, name);

    if (system_packages)
	channel->setSystem (true);

    channel->setType (chan_type);

    store->addChannel (channel);

    if (chan_type == CHANNEL_TYPE_HELIX) {
	count = extract_packages_from_helix_file (pathname, channel, add_to_world_cb, (void *)&store);
    } else if (chan_type == CHANNEL_TYPE_DEBIAN) {
	count = extract_packages_from_debian_file (pathname, channel, add_to_world_cb, (void *)&store);
    } else {
	fprintf (stderr, "Unsupported channel type\n");
	return;
    }

    if (system_packages) {
	store->foreachResItem (channel, mark_as_system_cb, NULL);
    }

    printf ("Loaded %d package%s from %s\n", count, count == 1 ? "" : "s", pathname.c_str()); fflush (stdout);
}


static bool done_setup = false;

static void
parse_xml_setup (XmlNodePtr node)
{
    assertExit (node->equals("setup"));

    if (done_setup) {
	fprintf (stderr, "Multiple <setup>..</setup> sections not allowed!\n");
	exit (0);
    }
    done_setup = true;

    node = node->children();
    while (node != NULL) {
	if (!node->isElement()) {
	    node = node->next();
	    continue;
	}

	if (node->equals ("system")) {
	    const char *file = node->getProp ("file");
	    assertExit (file);
	    load_channel ("@system", file, "helix", true);
	    free ((void *)file);
	} else if (node->equals ("channel")) {
	    string name = node->getProp ("name");
	    string file = node->getProp ("file");
	    string type = node->getProp ("type");
	    assertExit (!name.empty());
	    assertExit (!file.empty());
	    load_channel (name, file, type, false);
	} else if (node->equals ("undump")) {
	    const char *file = node->getProp ("file");
	    assertExit (file);
	    undump (file);
	    free((void *)file);
	} else if (node->equals ("force-install")) {
	    const char *channel_name = node->getProp ("channel");
	    const char *package_name = node->getProp ("package");
	    constResItemPtr resItem;
	    constChannelPtr system_channel;

	    assertExit (channel_name);
	    assertExit (package_name);

	    resItem = get_resItem (channel_name, package_name);
	    if (resItem) {
		printf (">!> Force-installing %s from channel %s\n", package_name, channel_name);

		system_channel = world->getChannelById ("@system");

		if (!system_channel)
		    fprintf (stderr, "No system channel available!\n");

		ResItemPtr r = ResItemPtr::cast_away_const(resItem);
		r->setChannel (system_channel);
		r->setInstalled (true);
	    } else {
		fprintf (stderr, "Unknown package %s::%s\n", channel_name, package_name);
	    }

	    free ((void *)channel_name);
	    free ((void *)package_name);
	} else if (node->equals ("force-uninstall")) {
	    const char *package_name = node->getProp ("package");
	    constResItemPtr resItem;

	    assertExit (package_name);
	    resItem = get_resItem ("@system", package_name);
	    
	    if (! resItem) {
		fprintf (stderr, "Can't force-uninstall installed package '%s'\n", package_name);
	    } else {
		printf (">!> Force-uninstalling '%s'\n", package_name);
	    }

	    free ((void *)package_name);
	} else if (node->equals ("lock")) {
	    const char *channel_name = node->getProp ("channel");
	    const char *package_name = node->getProp ("package");
	    constResItemPtr resItem;

	    assertExit (channel_name);
	    assertExit (package_name);

	    resItem = get_resItem (channel_name, package_name);
	    if (resItem) {
		printf (">!> Locking %s from channel %s\n", package_name, channel_name);
		ResItemPtr r = ResItemPtr::cast_away_const(resItem);
		r->setLocked (true);
	    } else {
		fprintf (stderr, "Unknown package %s::%s\n", channel_name, package_name);
	    }

	    free ((void *)channel_name);
	    free ((void *)package_name);

	} else {
	    fprintf (stderr, "Unrecognized tag '%s' in setup\n", node->name());
	}

	node = node->next();
    }
}

//---------------------------------------------------------------------------------------------------------------------
// trial related functions

static void
report_solutions (Resolver & resolver)
{
    int count = 1;
    ChecksumList checksum_list;

    printf ("\n");

    if (!resolver.completeQueues().empty()) {
	printf ("Completed solutions: %ld\n", (long) resolver.completeQueues().size());
    }

    if (resolver.prunedQueues().empty()) {
	printf ("Pruned solutions: %ld\n", (long) resolver.prunedQueues().size());
    }

    if (resolver.deferredQueues().empty()) {
	printf ("Deferred solutions: %ld\n", (long) resolver.deferredQueues().size());
    }

    if (resolver.invalidQueues().empty()) {
	printf ("Invalid solutions: %ld\n", (long) resolver.invalidQueues().size());
    }
    
    if (resolver.bestContext()) {
	printf ("\nBest Solution:\n\n");
	print_solution (resolver.bestContext(), &count, checksum_list);

	ResolverQueueList complete = resolver.completeQueues();
	if (complete.size() > 1)
	    printf ("\nOther Valid Solutions:\n\n");

	if (complete.size() < 20) {
	    for (ResolverQueueList::const_iterator iter = complete.begin(); iter != complete.end(); iter++) {
		ResolverQueuePtr queue = (*iter);
		if (queue->context() != resolver.bestContext()) 
		    print_solution (queue->context(), &count, checksum_list);
	    }
	}
    }

    ResolverQueueList invalid = resolver.invalidQueues();
    if (invalid.size() < 20) {
	printf ("\n");

	for (ResolverQueueList::const_iterator iter = invalid.begin(); iter != invalid.end(); iter++) {
	    ResolverQueuePtr queue = (*iter);
	    printf ("Failed Solution: \n%s\n", queue->context()->asString().c_str());
	    printf ("- - - - - - - - - -\n");
	    queue->context()->spewInfo ();
	    fflush (stdout);
	}
    } else {
	printf ("(Not displaying more than 20 invalid solutions)\n");
    }
    fflush (stdout);
}


static bool
trial_upgrade_cb (constResItemPtr original, constResItemPtr upgrade, void *user_data)
{
    Resolver *resolver = (Resolver *)user_data;

    resolver->addResItemToInstall (upgrade);

    printf (">!> Upgrading %s => %s\n", original->asString().c_str(), upgrade->asString().c_str());

    return false;
}


static void
parse_xml_trial (XmlNodePtr node)
{
    bool verify = false;

    assertExit (node->equals ("trial"));

    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "parse_xml_setup()\n");

    if (! done_setup) {
	fprintf (stderr, "Any trials must be preceeded by the setup!\n");
	exit (0);
    }

    print_sep ();

    Resolver resolver;
    resolver.setWorld (world);

    node = node->children();
    while (node) {
	if (!node->isElement()) {
	    node = node->next();
	    continue;
	}

	if (node->equals("note")) {

	    const char *note = node->getContent ();
	    printf ("NOTE: %s\n", note);
	    free ((void *)note);

	} else if (node->equals ("verify")) {

	    verify = true;

	} else if (node->equals ("current")) {

	    const char *channel_name = node->getProp ("channel");
	    constChannelPtr channel = get_channel (channel_name);

	    if (channel != NULL) {
		resolver.setCurrentChannel (channel);
	    } else {
		fprintf (stderr, "Unknown channel '%s' (current)\n", channel_name);
	    }

	    free ((void *)channel_name);

	} else if (node->equals ("subscribe")) {

	    const char *channel_name = node->getProp ("channel");
	    ChannelPtr channel = get_channel (channel_name);

	    if (channel != NULL) {
		channel->setSubscription (true);
	    } else {
		fprintf (stderr, "Unknown channel '%s' (subscribe)\n", channel_name);
	    }

	    free ((void *)channel_name);
	
	} else if (node->equals ("install")) {

	    const char *channel_name = node->getProp ("channel");
	    const char *package_name = node->getProp ("package");
	    constResItemPtr resItem;

	    assertExit (channel_name);
	    assertExit (package_name);

	    resItem = get_resItem (channel_name, package_name);
	    if (resItem) {
		printf (">!> Installing %s from channel %s\n", package_name, channel_name);
		resolver.addResItemToInstall (resItem);
	    } else {
		fprintf (stderr, "Unknown package %s::%s\n", channel_name, package_name);
	    }

	    free ((void *)channel_name);
	    free ((void *)package_name);

	} else if (node->equals ("uninstall")) {

	    const char *package_name = node->getProp ("package");
	    constResItemPtr resItem;

	    assertExit (package_name);

	    resItem = get_resItem ("@system", package_name);
	    if (resItem) {
		printf (">!> Uninstalling %s\n", package_name);
		resolver.addResItemToRemove (resItem);
	    } else {
		fprintf (stderr, "Unknown system package %s\n", package_name);
	    }

	    free ((void *)package_name);

	} else if (node->equals ("upgrade")) {
	    int count;

	    printf (">!> Checking for upgrades...\n");

	    count = world->foreachSystemUpgrade (true, trial_upgrade_cb, (void *)&resolver);
	    
	    if (count == 0)
		printf (">!> System is up-to-date, no upgrades required\n");
	    else
		printf (">!> Upgrading %d package%s\n", count, count > 1 ? "s" : "");

	} else if (node->equals ("solvedeps")) {

	    XmlNodePtr iter = node->children();

	    while (iter != NULL) {
		DependencyPtr dep = new Dependency (iter);

		/* We just skip over anything that doesn't look like a dependency. */

		if (dep) {
		    const char *conflict_str = iter->getProp ("conflict");

		    printf (">!> Solvedeps %s%s\n", conflict_str ? "conflict " : "",  dep->asString().c_str());

		    resolver.addExtraDependency (dep);

		    free ((void *)conflict_str);
		}
		iter = iter->next();
	    }

	} else {
	    fprintf (stderr, "Unknown tag '%s' in trial\n", node->name());
	}

	node = node->next();
    }

    if (getenv ("RC_DEPS_TIME")) {
	int timeout = atoi (getenv ("RC_DEPS_TIME"));

	resolver.setTimeout (timeout);
    }

    if (verify)
	resolver.verifySystem ();
    else
	resolver.resolveDependencies ();

    report_solutions (resolver);
}

//---------------------------------------------------------------------------------------------------------------------

static void
parse_xml_test (XmlNodePtr node)
{
    assertExit (node->equals("test"));

    node = node->children();

    while (node) {
	if (node->type() == XML_ELEMENT_NODE) {
	    if (node->equals("setup")) {
		parse_xml_setup (node);
	    } else if (node->equals ("trial")) {
		parse_xml_trial (node);
	    } else {
		fprintf (stderr, "Unknown tag '%s' in test\n", node->name());
	    }
	}

	node = node->next();
    }
}


static void
process_xml_test_file (const char *filename)
{
    xmlDocPtr xml_doc;
    XmlNodePtr root;

    xml_doc = xmlParseFile (filename);
    if (xml_doc == NULL) {
	fprintf (stderr, "Can't parse test file '%s'\n", filename);
	exit (0);
    }

    root = new XmlNode (xmlDocGetRootElement (xml_doc));

    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "Parsing file '%s'\n", filename);
    
    parse_xml_test (root);
    
    xmlFreeDoc (xml_doc);
}


//---------------------------------------------------------------------------------------------------------------------

static void
init_libzypp (void)
{
//    rc_version_set_global (rc_version_rpm_new());		//  rpm is the default for GVersion

      World::setGlobalWorld (new MultiWorld());
}

int
main (int argc, char *argv[])
{
    if (argc != 2) {
	fprintf (stderr, "Usage: deptestomatic testfile.xml\n");
	exit (0);
    }

    init_libzypp ();

    world = World::globalWorld();

    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "init_libzypp() done\n");

    process_xml_test_file (argv[1]);

    return 0;
}

