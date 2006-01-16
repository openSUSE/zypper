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


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <zypp/Resolvable.h>
#include <zypp/ResTraits.h>
#include <zypp/Capability.h>
#include <zypp/CapSet.h>
#include <zypp/CapFactory.h>
#include <zypp/solver/libzypp_solver.h>

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
using zypp::Resolvable;
using zypp::ResTraits;
using zypp::Capability;
using zypp::CapSet;
using zypp::CapFactory;
using namespace zypp::solver::detail;

static MultiWorld_Ptr world = NULL;
static string globalPath;

typedef list<unsigned int> ChecksumList;

//---------------------------------------------------------------------------

Resolvable::Kind
string2kind (const std::string & str)
{
    Resolvable::Kind kind = ResTraits<zypp::Package>::kind;
    if (!str.empty()) {
	if (str == "patch") {
	    kind = ResTraits<zypp::Patch>::kind;
	}
	else if (str == "pattern") {
	    kind = ResTraits<zypp::Pattern>::kind;
	}
	else if (str == "script") {
	    kind = ResTraits<zypp::Script>::kind;
	}
	else if (str == "message") {
	    kind = ResTraits<zypp::Message>::kind;
	}
	else if (str == "product") {
	    kind = ResTraits<zypp::Product>::kind;
	}
	else {
	    fprintf (stderr, "get_resItem unknown kind '%s'\n", str.c_str());
	}
    }
    return kind;
}

//---------------------------------------------------------------------------

#if 0
static void
lock_resItem (ResItem_Ptr resItem)
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

#endif	// 0


typedef struct {
    ResItem_constPtr resolvable;
} RemoveTypeInfo;
 

static bool
remove_resItem_cb (World_Ptr world, void *data)
{
    RemoveTypeInfo *info = (RemoveTypeInfo *)data;
    StoreWorld_Ptr store = boost::dynamic_pointer_cast<StoreWorld>(world);
    if (store == NULL) {
	fprintf (stderr, "remove_resItem_cb: world is not a STORE_WORLD\n");
	return false;
    }
    store->removeResItem (info->resolvable);

    return true;
}


static void
remove_resItem (ResItem_constPtr resItem)
{
    RemoveTypeInfo info = { resItem };
    world->foreachSubworldByType (STORE_WORLD, remove_resItem_cb, &info);
}

/* ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** */


//==============================================================================================================================

//---------------------------------------------------------------------------------------------------------------------
// helper functions

typedef list<string> StringList;

static void
assemble_install_cb (ResItem_constPtr resItem,
		     ResItemStatus status,
		     void *data)
{
    StringList *slist = (StringList *)data;
    char buf[10];
    snprintf (buf, 10, "%-7s ", resItem->isInstalled() ? "|flag" : "install");
    string str (buf);
    str += resItem->asString();

    slist->push_back (str);
}


static void
assemble_uninstall_cb (ResItem_constPtr resItem,
		       ResItemStatus status,
		       void *data)
{
    StringList *slist = (StringList *)data;
    char buf[10];
    snprintf (buf, 10, "%-7s ", resItem->isInstalled() ? "remove" : "|unflag");
    string str (buf);
    str += resItem->asString();

    slist->push_back (str);
}


static void
assemble_upgrade_cb (ResItem_constPtr res1,
		     ResItemStatus status1,
		     ResItem_constPtr res2,
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
print_solution (ResolverContext_Ptr context, int *count, ChecksumList & checksum_list, bool instorder)
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

    printf (">!> installs=%d, upgrades=%d, uninstalls=%d", context->installCount(), context->upgradeCount(), context->uninstallCount());
    int satisfied = context->satisfyCount();
    if (satisfied > 0) printf (", satisfied=%d", satisfied);
    printf ("\n");
    printf ("download size=%.1fk, install size=%.1fk\n", context->downloadSize() / 1024.0, context->installSize() / 1024.0);
    printf ("total priority=%d, min priority=%d, max priority=%d\n", context->totalPriority(), context->minPriority(), context->maxPriority());
    printf ("other penalties=%d\n",  context->otherPenalties());
    printf ("- - - - - - - - - -\n");

    if (instorder) {
	printf ("\n>!> Installation Order:\n\n");
	CResItemList installs = context->getMarkedResItems(1);
	CResItemList dummy;

	InstallOrder order( world, installs, dummy );		 // sort according top prereq
	order.init();
	const CResItemList & installorder ( order.getTopSorted() );
	for (CResItemList::const_iterator iter = installorder.begin(); iter != installorder.end(); iter++) {
		printf (">!> %s\n", (*iter)->asString().c_str());
	}
	printf ("- - - - - - - - - -\n");
    }

    fflush (stdout);

    context->spewInfo ();
    if (getenv ("RC_SPEW")) printf ("%s\n", context->asString().c_str());
    fflush (stdout);

}


//---------------------------------------------------------------------------------------------------------------------
static void
undump (const std::string & filename)
{
    UndumpWorld_Ptr undump_world;
    std::string pathname = globalPath + filename;

    undump_world = new UndumpWorld (pathname);
    if (undump_world == NULL) {
	fprintf (stderr, "Couldn't undump from file '%s'", pathname.c_str());
	return;
    }

    world->addSubworld (undump_world);
}


static Channel_Ptr
get_channel (const string & channel_name)
{
    Channel_Ptr channel;

    channel = world->getChannelById (channel_name);

    if (channel == NULL)
	channel = world->getChannelByAlias (channel_name);

    if (channel == NULL)
	channel = world->getChannelByName (channel_name);

    return channel;
}


static ResItem_constPtr
get_resItem (const string & channel_name, const string & package_name, const string & kind_name = "")
{
    Channel_constPtr channel;
    ResItem_constPtr resItem;
    Resolvable::Kind kind = string2kind (kind_name);
    channel = get_channel (channel_name);

    if (channel == NULL) {
	fprintf (stderr, "Can't find resolvable '%s': channel '%s' not defined\n", package_name.c_str(), channel_name.c_str());
	return NULL;
    }

    resItem = world->findResItem (channel, package_name, kind);

    if (resItem == NULL) {
	fprintf (stderr, "Can't find resolvable '%s' in channel '%s': no such name/kind\n", package_name.c_str(), channel_name.c_str());
	return NULL;
    }

    return resItem;
}

//---------------------------------------------------------------------------------------------------------------------
// whatdependson

typedef struct {
    CResItemSet *rs;
    ResItem_constPtr resItem;
    Capability cap;
    bool first;
} WhatDependsOnInfo;


static bool
requires_resItem_cb (ResItem_constPtr resItem, const Capability & cap, void *data)
{
    WhatDependsOnInfo *info = (WhatDependsOnInfo *)data;
    if (info->rs->insert (resItem).second) {
	if (info->first) {
    printf ("\t%s provides %s required by\n", info->resItem->asString().c_str(), info->cap.asString().c_str());
	    info->first = false;
	}
	printf ("\t\t%s for %s\n", resItem->asString().c_str(), cap.asString().c_str());
    }
    return true;
}


static CResItemSet
whatdependson (ResItem_constPtr resItem)
{
    CResItemSet rs;

    printf ("\n\nWhat depends on '%s'\n", resItem->asString().c_str());

    WhatDependsOnInfo info;
    info.rs = &rs;
    info.resItem = resItem;

    // loop over all provides and call foreachRequiringResItem
    CapSet caps = resItem->provides();
    for (CapSet::const_iterator cap_iter = caps.begin(); cap_iter != caps.end(); ++cap_iter) {
	info.cap = *cap_iter;
	info.first = true;
	world->foreachRequiringResItem (info.cap, requires_resItem_cb, &info);
    }

    return rs;
}

//---------------------------------------------------------------------------------------------------------------------
// whatprovides

static bool
providing_resItem_cb (ResItem_constPtr resItem, const Capability & cap, void *data)
{
    CResItemSet *rs = (CResItemSet *)data;
    rs->insert (resItem);
    return true;
}


static CResItemSet
get_providing_resItems (const string & prov_name, const string & kind_name = "")
{
    CResItemSet rs;
    Resolvable::Kind kind = string2kind (kind_name);

    CapFactory factory;
    Capability cap = factory.parse (kind, prov_name);
    world->foreachProvidingResItem (cap, providing_resItem_cb, &rs);

    return rs;
}

//---------------------------------------------------------------------------------------------------------------------
// setup related functions

static bool
add_to_world_cb (ResItem_constPtr resItem, void *data)
{
    StoreWorld_Ptr world = *((StoreWorld_Ptr *)data);
    world->addResItem (resItem);

    return true;
}


static void
load_channel (const string & name, const string & filename, const string & type, bool system_packages)
{
    string pathname = globalPath + filename;

    if (getenv ("RC_SPEW")) fprintf (stderr, "load_channel(%s,%s,%s,%s)\n", name.c_str(), pathname.c_str(), type.c_str(), system_packages?"system":"non-system");

    ChannelType chan_type = system_packages ? CHANNEL_TYPE_SYSTEM : CHANNEL_TYPE_UNKNOWN;
    Channel_Ptr channel;
    unsigned int count;
    StoreWorld_Ptr store = new StoreWorld();

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
	fprintf (stderr, "Unsupported channel 'debian'\n");
	exit (1);
//	count = extract_packages_from_debian_file (pathname, channel, add_to_world_cb, (void *)&store);
    } else {
	fprintf (stderr, "Unsupported channel type\n");
	return;
    }

    printf ("Loaded %d package%s from %s\n", count, count == 1 ? "" : "s", pathname.c_str()); fflush (stdout);
}


static bool done_setup = false;

static void
parse_xml_setup (XmlNode_Ptr node)
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

	    string file = node->getProp ("file");
	    assertExit (!file.empty());
	    load_channel ("@system", file, "helix", true);

	} else if (node->equals ("channel")) {

	    string name = node->getProp ("name");
	    string file = node->getProp ("file");
	    string type = node->getProp ("type");
	    assertExit (!name.empty());
	    assertExit (!file.empty());
	    load_channel (name, file, type, false);

	} else if (node->equals ("undump")) {

	    string file = node->getProp ("file");
	    assertExit (!file.empty());
	    undump (file);

	} else if (node->equals ("force-install")) {

	    string channel_name = node->getProp ("channel");
	    string package_name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");

	    ResItem_constPtr resItem;
	    Channel_constPtr system_channel;

	    assertExit (!channel_name.empty());
	    assertExit (!package_name.empty());

	    resItem = get_resItem (channel_name, package_name, kind_name);
	    if (resItem) {
		printf (">!> Force-installing %s from channel %s\n", package_name.c_str(), channel_name.c_str());

		system_channel = world->getChannelById ("@system");

		if (!system_channel)
		    fprintf (stderr, "No system channel available!\n");
#warning force-install disabled
//		ResItem_Ptr r = ResItem_Ptr::cast_away_const(resItem);
//		r->setChannel (system_channel);
//		r->setInstalled (true);
	    } else {
		fprintf (stderr, "Unknown package %s::%s\n", channel_name.c_str(), package_name.c_str());
	    }

	} else if (node->equals ("force-uninstall")) {

	    string package_name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");

	    ResItem_constPtr resItem;

	    assertExit (!package_name.empty());
	    resItem = get_resItem ("@system", package_name, kind_name);
	    
	    if (! resItem) {
		fprintf (stderr, "Can't force-uninstall installed package '%s'\n", package_name.c_str());
	    } else {
		printf (">!> Force-uninstalling '%s'\n", package_name.c_str());
		remove_resItem (resItem);
	    }

	} else if (node->equals ("lock")) {

	    string channel_name = node->getProp ("channel");
	    string package_name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");

	    ResItem_constPtr resItem;

	    assertExit (!channel_name.c_str());
	    assertExit (!package_name.c_str());

	    resItem = get_resItem (channel_name, package_name, kind_name);
	    if (resItem) {
		printf (">!> Locking %s from channel %s\n", package_name.c_str(), channel_name.c_str());
#warning lock disabled
//		ResItem_Ptr r = ResItem_Ptr::cast_away_const(resItem);
//		r->setLocked (true);
	    } else {
		fprintf (stderr, "Unknown package %s::%s\n", channel_name.c_str(), package_name.c_str());
	    }

	} else {
	    fprintf (stderr, "Unrecognized tag '%s' in setup\n", node->name().c_str());
	}

	node = node->next();
    }
}

//---------------------------------------------------------------------------------------------------------------------
// trial related functions

static void
report_solutions (Resolver & resolver, bool instorder)
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
	print_solution (resolver.bestContext(), &count, checksum_list, instorder);

	ResolverQueueList complete = resolver.completeQueues();
	if (complete.size() > 1)
	    printf ("\nOther Valid Solutions:\n\n");

	if (complete.size() < 20) {
	    for (ResolverQueueList::const_iterator iter = complete.begin(); iter != complete.end(); iter++) {
		ResolverQueue_Ptr queue = (*iter);
		if (queue->context() != resolver.bestContext()) 
		    print_solution (queue->context(), &count, checksum_list, instorder);
	    }
	}
    }

    ResolverQueueList invalid = resolver.invalidQueues();
    if (invalid.size() < 20) {
	printf ("\n");

	for (ResolverQueueList::const_iterator iter = invalid.begin(); iter != invalid.end(); iter++) {
	    ResolverQueue_Ptr queue = (*iter);
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
trial_upgrade_cb (ResItem_constPtr original, ResItem_constPtr upgrade, void *user_data)
{
    Resolver *resolver = (Resolver *)user_data;

    resolver->addResItemToInstall (upgrade);

    printf (">!> Upgrading %s => %s\n", original->asString().c_str(), upgrade->asString().c_str());

    return false;
}


static void
parse_xml_trial (XmlNode_Ptr node)
{
    bool verify = false;
    bool instorder = false;

    assertExit (node->equals ("trial"));

    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "parse_xml_setup()\n");

    if (! done_setup) {
	fprintf (stderr, "Any trials must be preceeded by the setup!\n");
	exit (0);
    }

    print_sep ();

    Resolver resolver;
    resolver.setWorld (world);
    ResolverContext_Ptr established = NULL;

    node = node->children();
    while (node) {
	if (!node->isElement()) {
	    node = node->next();
	    continue;
	}

	if (node->equals("note")) {

	    string note = node->getContent ();
	    printf ("NOTE: %s\n", note.c_str());

	} else if (node->equals ("verify")) {

	    verify = true;

	} else if (node->equals ("current")) {

	    string channel_name = node->getProp ("channel");
	    Channel_constPtr channel = get_channel (channel_name);

	    if (channel != NULL) {
		resolver.setCurrentChannel (channel);
	    } else {
		fprintf (stderr, "Unknown channel '%s' (current)\n", channel_name.c_str());
	    }

	} else if (node->equals ("subscribe")) {

	    string channel_name = node->getProp ("channel");
	    Channel_Ptr channel = get_channel (channel_name);

	    if (channel != NULL) {
		channel->setSubscription (true);
	    } else {
		fprintf (stderr, "Unknown channel '%s' (subscribe)\n", channel_name.c_str());
	    }

	} else if (node->equals ("install")) {

	    string channel_name = node->getProp ("channel");
	    string package_name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");

	    ResItem_constPtr resItem;

	    assertExit (!channel_name.empty());
	    assertExit (!package_name.empty());

	    resItem = get_resItem (channel_name, package_name, kind_name);
	    if (resItem) {
		printf (">!> Installing %s from channel %s\n", package_name.c_str(), channel_name.c_str());
		resolver.addResItemToInstall (resItem);
	    } else {
		fprintf (stderr, "Unknown package %s::%s\n", channel_name.c_str(), package_name.c_str());
	    }

	} else if (node->equals ("uninstall")) {

	    string package_name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");

	    ResItem_constPtr resItem;

	    assertExit (!package_name.empty());

	    resItem = get_resItem ("@system", package_name, kind_name);
	    if (resItem) {
		printf (">!> Uninstalling %s\n", package_name.c_str());
		resolver.addResItemToRemove (resItem);
	    } else {
		fprintf (stderr, "Unknown system package %s\n", package_name.c_str());
	    }

	} else if (node->equals ("upgrade")) {
	    int count;

	    printf (">!> Checking for upgrades...\n");

	    count = world->foreachSystemUpgrade (true, trial_upgrade_cb, (void *)&resolver);
	    
	    if (count == 0)
		printf (">!> System is up-to-date, no upgrades required\n");
	    else
		printf (">!> Upgrading %d package%s\n", count, count > 1 ? "s" : "");

	} else if (node->equals ("establish")) {

	    printf (">!> Establishing state ...\n");

	    resolver.establishState ();

	    established = resolver.bestContext();
	    if (established == NULL)
		printf (">!> Established NO context !\n");
	    else
		printf (">!> Established context\n\t%s\n", established->asString().c_str());

	} else if (node->equals ("instorder")) {

	    printf (">!> Calculating installation order ...\n");

	    instorder = true;

	} else if (node->equals ("solvedeps")) {
#if 0
	    XmlNode_Ptr iter = node->children();

	    while (iter != NULL) {
		Dependency_Ptr dep = new Dependency (iter);

		/* We just skip over anything that doesn't look like a dependency. */

		if (dep) {
		    string conflict_str = iter->getProp ("conflict");

		    printf (">!> Solvedeps %s%s\n", conflict_str.empty() ? "" : "conflict ",  dep->asString().c_str());

		    resolver.addExtraDependency (dep);

		}
		iter = iter->next();
	    }
#else
#warning solvedeps disabled
#endif

	} else if (node->equals ("whatprovides")) {

	    string kind_name = node->getProp ("kind");
	    string prov_name = node->getProp ("provides");

	    CResItemSet resItems;

	    printf ("resItems providing '%s'\n", prov_name.c_str());

	    resItems = get_providing_resItems (prov_name, kind_name);

	    if (resItems.empty()) {
		fprintf (stderr, "None found\n");
	    } else {
		for (CResItemSet::const_iterator iter = resItems.begin(); iter != resItems.end(); ++iter) {
		    printf ("%s\n", (*iter)->asString().c_str());
		}
	    }

	} else if (node->equals ("whatdependson")) {

	    string channel_name = node->getProp ("channel");
	    string package_name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");
	    string prov_name = node->getProp ("provides");

	    CResItemSet resItems;

	    assert (!channel_name.empty());
	    assert (!package_name.empty());

	    if (!prov_name.empty()) {
		if (!package_name.empty()) {
		    fprintf (stderr, "<whatdependson ...> can't have both package and provides.\n");
		    exit (1);
		}
		resItems = get_providing_resItems (prov_name, kind_name);
	    }
	    else {
		ResItem_constPtr resItem = get_resItem (channel_name, package_name, kind_name);
		if (resItem) resItems.insert (resItem);
	    }
	    if (resItems.empty()) {
		fprintf (stderr, "Can't find matching package\n");
	    } else {
		for (CResItemSet::const_iterator iter = resItems.begin(); iter != resItems.end(); ++iter) {
		    CResItemSet dependants = whatdependson (*iter);
		    for (CResItemSet::const_iterator dep_iter = dependants.begin(); dep_iter != dependants.end(); ++dep_iter) {
			printf ("%s\n", (*dep_iter)->asString().c_str());
		    }
		}
	    }

	} else {
	    fprintf (stderr, "Unknown tag '%s' in trial\n", node->name().c_str());
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
	resolver.resolveDependencies (established);

    report_solutions (resolver, instorder);
}

//---------------------------------------------------------------------------------------------------------------------

static void
parse_xml_test (XmlNode_Ptr node)
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
		fprintf (stderr, "Unknown tag '%s' in test\n", node->name().c_str());
	    }
	}

	node = node->next();
    }
}


static void
process_xml_test_file (const string & filename)
{
    xmlDocPtr xml_doc;
    XmlNode_Ptr root;

    xml_doc = xmlParseFile (filename.c_str());
    if (xml_doc == NULL) {
	fprintf (stderr, "Can't parse test file '%s'\n", filename.c_str());
	exit (0);
    }

    root = new XmlNode (xmlDocGetRootElement (xml_doc));

    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "Parsing file '%s'\n", filename.c_str());
    
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
    setenv("ZYPP_NOLOG","1",1); // no logging
    
    if (argc != 2) {
	fprintf (stderr, "Usage: deptestomatic testfile.xml\n");
	exit (0);
    }

    init_libzypp ();

    globalPath = argv[1];
    globalPath = globalPath.substr (0, globalPath.find_last_of ("/") +1);

    world = World::globalWorld();

    if (getenv ("RC_SPEW_XML")) fprintf (stderr, "init_libzypp() done\n");

    process_xml_test_file (string (argv[1]));

    return 0;
}

