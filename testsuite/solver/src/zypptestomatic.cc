/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * zypptestomatic.cc
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


#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <zypp/Resolvable.h>
#include <zypp/ResTraits.h>
#include <zypp/ResPool.h>
#include <zypp/PoolItem.h>
#include <zypp/Capability.h>
#include <zypp/CapSet.h>
#include <zypp/CapFactory.h>
#include <zypp/solver/libzypp_solver.h>

#include <sstream>
#include <iostream>
#include <zypp/base/String.h>

using namespace std;
using namespace zypp;
using namespace zypp::solver::detail;

int assertOutput( const char* output)
{
    cout << "Assertion in " << __FILE__ << ", " << __LINE__ << " : " << output << " --> exit" << endl;
    exit (0);
}

# define assertExit(expr) \
  (__ASSERT_VOID_CAST ((expr) ? 0 :		 \
		       (assertOutput (__STRING(expr)))))

static string globalPath;
static ResPool *globalPool;

typedef list<unsigned int> ChecksumList;

#define RESULT cout << ">!> "

//---------------------------------------------------------------------------

Resolvable::Kind
string2kind (const std::string & str)
{
    Resolvable::Kind kind = ResTraits<zypp::Package>::kind;
    if (!str.empty()) {
	if (str == "package") {
	    // empty 
	}
	else if (str == "patch") {
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
	    cerr << "get_poolItem unknown kind '" << str << "'" << endl;
	}
    }
    return kind;
}

//---------------------------------------------------------------------------

#warning Locks not implemented
#if 0
static void
lock_poolItem (PoolItem_Ref poolItem)
{
    RCResItemDep *dep;
    RCResItemMatch *match;

    dep = rc_poolItem_dep_new_from_spec (RC_RESOLVABLE_SPEC (poolItem),
					RC_RELATION_EQUAL, RC_TYPE_RESOLVABLE,
					RC_CHANNEL_ANY, false, false);

    match = rc_poolItem_match_new ();
    rc_poolItem_match_set_dep (match, dep);

    rc_poolItem_dep_unref (dep);

    rc_world_add_lock (rc_get_world (), match);
}

#endif	// 0

/* ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** */

//==============================================================================================================================

//---------------------------------------------------------------------------------------------------------------------
// helper functions

typedef list<string> StringList;

static void
assemble_install_cb (PoolItem_Ref poolItem,
		     void *data)
{
    StringList *slist = (StringList *)data;
    ostringstream s;
    s << str::form ("%-7s ", poolItem.status().isInstalled() ? "|flag" : "install");
    s << poolItem.resolvable();

    slist->push_back (s.str());
}


static void
assemble_uninstall_cb (PoolItem_Ref poolItem,
		       void *data)
{
    StringList *slist = (StringList *)data;
    ostringstream s;
    s << str::form ("%-7s ", poolItem.status().isInstalled() ? "remove" : "|unflag");
    s << poolItem.resolvable();

    slist->push_back (s.str());
}


static void
assemble_upgrade_cb (PoolItem_Ref res1,
		     PoolItem_Ref res2,
		     void *data)
{
    StringList *slist = (StringList *)data;
    ostringstream s;

    s << "upgrade ";

    s << res2.resolvable();
    s << " => ";
    s << res1.resolvable();

    slist->push_back (s.str());
}


static void
assemble_incomplete_cb (PoolItem_Ref poolItem,
		       void *data)
{
    StringList *slist = (StringList *)data;
    ostringstream s;
    s << str::form ("%-11s ", poolItem.status().isInstalled() ? "incomplete" : "|incomplete");
    s << poolItem.resolvable();

    slist->push_back (s.str());
}


static void
assemble_satisfy_cb (PoolItem_Ref poolItem,
		       void *data)
{
    StringList *slist = (StringList *)data;
    ostringstream s;
    s << str::form ("%-10s ", poolItem.status().isInstalled() ? "SATISFIED" : "|satisfied");
    s << poolItem.resolvable();

    slist->push_back (s.str());
}

static void
print_sep (void)
{
    cout << endl << "------------------------------------------------" << endl << endl;
}


static void
print_important (const string & str)
{
    RESULT << str.c_str() << endl;
}


static void
print_solution (ResolverContext_Ptr context, int *count, ChecksumList & checksum_list, bool instorder)
{
    if (context->isValid ()) {

	StringList items;
	items.clear();

	unsigned int checksum = 0;
	bool is_dup = false;

	RESULT << "Solution #" << *count << ":" << endl;
	++*count;

	context->foreachInstall (assemble_install_cb, &items);

	context->foreachUninstall (assemble_uninstall_cb, &items);

	context->foreachUpgrade (assemble_upgrade_cb, &items);

	context->foreachIncomplete (assemble_incomplete_cb, &items);

	context->foreachSatisfy (assemble_satisfy_cb, &items);

	items.sort ();

	for (StringList::const_iterator iter = items.begin(); iter != items.end(); iter++) {
	    const char *c = (*iter).c_str();
	    while (*c) {
		checksum = 17 * checksum + (unsigned int)*c;
		++c;
	    }
	}
	cout << str::form ("Checksum = %x", checksum) << endl;

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
	    RESULT << "This solution is a duplicate." << endl;
	}

	items.clear();

    } else {
	RESULT << "Failed Attempt:" << endl;
    }

    RESULT << "installs=" << context->installCount() << ", upgrades=" << context->upgradeCount() << ", uninstalls=" << context->uninstallCount();
    int satisfied = context->satisfyCount();
    if (satisfied > 0) cout << ", satisfied=" << satisfied;
    cout << endl;
    cout << str::form ("download size=%.1fk, install size=%.1fk\n", context->downloadSize() / 1024.0, context->installSize() / 1024.0);
    cout << str::form ("total priority=%d, min priority=%d, max priority=%d\n", context->totalPriority(), context->minPriority(), context->maxPriority());
    cout << str::form ("other penalties=%d\n",  context->otherPenalties());
    cout << "- - - - - - - - - -" << endl;

    if (instorder) {
	cout << endl;
	RESULT << "Installation Order:" << endl << endl;
	PoolItemList installs = context->getMarked(1);
	PoolItemList dummy;

	InstallOrder order( globalPool, installs, dummy );		 // sort according top prereq
	order.init();
	const PoolItemList & installorder ( order.getTopSorted() );
	for (PoolItemList::const_iterator iter = installorder.begin(); iter != installorder.end(); iter++) {
		RESULT << (*iter) << endl;
	}
	cout << "- - - - - - - - - -" << endl;
    }

    fflush (stdout);

    context->spewInfo ();
    if (getenv ("RC_SPEW")) cout << context << endl;

}

#if 0

//---------------------------------------------------------------------------------------------------------------------
#if 0
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
#endif

static PoolItem_Ref
get_poolItem (const string & channel_name, const string & package_name, const string & kind_name = "")
{
    Channel_constPtr channel;
    PoolItem_Ref poolItem;
    Resolvable::Kind kind = string2kind (kind_name);
    channel = get_channel (channel_name);

    if (channel == NULL) {
	cerr << "Can't find resolvable '" << package_name << "': channel '" << channel_name << "' not defined" << endl;
	return NULL;
    }

    poolItem = world->findResItem (channel, package_name, kind);

    if (poolItem == NULL) {
	cerr << "Can't find resolvable '" << package_name << "' in channel '" << channel_name << "': no such name/kind" << endl;
	return NULL;
    }

    return poolItem;
}

//---------------------------------------------------------------------------------------------------------------------
// whatdependson

typedef struct {
    PoolItemSet *rs;
    PoolItem_Ref poolItem;
    Capability cap;
    bool first;
} WhatDependsOnInfo;


static bool
requires_poolItem_cb (PoolItem_Ref poolItem, const Capability & cap, void *data)
{
    WhatDependsOnInfo *info = (WhatDependsOnInfo *)data;
    if (info->rs->insert (poolItem).second) {
	if (info->first) {
	    cout << "\t" << info->poolItem.resolvable() << " provides " << info->cap << " required by" << endl;
	    info->first = false;
	}
	cout << "\t\t" << poolItem.resolvable() << " for " << cap << endl;
    }
    return true;
}


static PoolItemSet
whatdependson (PoolItem_Ref poolItem)
{
    PoolItemSet rs;

    cout << endl << endl << "What depends on '" << poolItem.resolvable() << "'" << endl;

    WhatDependsOnInfo info;
    info.rs = &rs;
    info.poolItem = poolItem;

    // loop over all provides and call foreachRequiringResItem
    CapSet caps = poolItem->provides();
    for (CapSet::const_iterator cap_iter = caps.begin(); cap_iter != caps.end(); ++cap_iter) {
	info.cap = *cap_iter;
	info.first = true;
	world->foreachRequiringResItem (info.cap, requires_poolItem_cb, &info);
    }

    return rs;
}

//---------------------------------------------------------------------------------------------------------------------
// whatprovides

static bool
providing_poolItem_cb (PoolItem_Ref poolItem, const Capability & cap, void *data)
{
    PoolItemSet *rs = (PoolItemSet *)data;
    rs->insert (poolItem);
    return true;
}


static PoolItemSet
get_providing_poolItems (const string & prov_name, const string & kind_name = "")
{
    PoolItemSet rs;
    Resolvable::Kind kind = string2kind (kind_name);

    CapFactory factory;
    Capability cap = factory.parse (kind, prov_name);
    world->foreachProvidingResItem (cap, providing_poolItem_cb, &rs);

    return rs;
}

//---------------------------------------------------------------------------------------------------------------------
// setup related functions

static bool
add_to_world_cb (PoolItem_Ref poolItem, void *data)
{
    ResPool *pool = *((StoreWorld_Ptr *)data);
    world->addPoolItem (poolItem);

    return true;
}


static void
load_channel (const string & name, const string & filename, const string & type, bool system_packages)
{
    string pathname = globalPath + filename;

    ChannelType chan_type = system_packages ? CHANNEL_TYPE_SYSTEM : CHANNEL_TYPE_UNKNOWN;
    Channel_Ptr channel;
    unsigned int count;
    StoreWorld_Ptr store = new StoreWorld();

    World::globalWorld()->addSubworld (store);

    if (type == "helix")
	chan_type = CHANNEL_TYPE_HELIX;
    else if (type == "debian")
	chan_type = CHANNEL_TYPE_DEBIAN;

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
	cerr << "Unsupported channel 'debian'" << endl;
	exit (1);
//	count = extract_packages_from_debian_file (pathname, channel, add_to_world_cb, (void *)&store);
    } else {
	cerr << "Unsupported channel type" << endl;
	return;
    }

    cout << "Loaded " << count << " package(s) from " << pathname << endl;
}


static bool done_setup = false;

static void
parse_xml_setup (XmlNode_Ptr node)
{
    assertExit (node->equals("setup"));

    if (done_setup) {
	cerr << "Multiple <setup>..</setup> sections not allowed!" << endl;
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

	    PoolItem_Ref poolItem;
	    Channel_constPtr system_channel;

	    assertExit (!channel_name.empty());
	    assertExit (!package_name.empty());

	    poolItem = get_poolItem (channel_name, package_name, kind_name);
	    if (poolItem) {
		RESULT << "Force-installing " << package_name << " from channel " << channel_name << endl;;

		system_channel = world->getChannelById ("@system");

		if (!system_channel)
		    cerr << "No system channel available!" << endl;
		PoolItem_Ref r = boost::const_pointer_cast<ResItem>(poolItem);
		r->setChannel (system_channel);
		r->setInstalled (true);
	    } else {
		cerr << "Unknown package %s::%s\n", channel_name.c_str(), package_name.c_str());
	    }

	} else if (node->equals ("force-uninstall")) {

	    string package_name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");

	    PoolItem_Ref poolItem;

	    assertExit (!package_name.empty());
	    poolItem = get_poolItem ("@system", package_name, kind_name);
	    
	    if (! poolItem) {
		cerr << "Can't force-uninstall installed package '%s'\n", package_name.c_str());
	    } else {
		RESULT << "Force-uninstalling " << package_name << endl;
		globalPool->remove (poolItem);
	    }

	} else if (node->equals ("lock")) {

	    string channel_name = node->getProp ("channel");
	    string package_name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");

	    PoolItem_Ref poolItem;

	    assertExit (!channel_name.empty());
	    assertExit (!package_name.empty());

	    poolItem = get_poolItem (channel_name, package_name, kind_name);
	    if (poolItem) {
		RESULT << "Locking " << package_name << " from channel " << channel_name << endl;
#warning Needs locks
#if 0
		r->setLocked (true);
#endif
	    } else {
		cerr << "Unknown package %s::%s\n", channel_name.c_str(), package_name.c_str());
	    }

	} else {
	    cerr << "Unrecognized tag '%s' in setup\n", node->name().c_str());
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

    printf ("" << endl;

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
	printf ("\nBest Solution:\n" << endl;
	print_solution (resolver.bestContext(), &count, checksum_list, instorder);

	ResolverQueueList complete = resolver.completeQueues();
	if (complete.size() > 1)
	    printf ("\nOther Valid Solutions:\n" << endl;

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
	printf ("" << endl;

	for (ResolverQueueList::const_iterator iter = invalid.begin(); iter != invalid.end(); iter++) {
	    ResolverQueue_Ptr queue = (*iter);
	    printf ("Failed Solution: \n%s\n", queue->context()->asString().c_str());
	    printf ("- - - - - - - - - -" << endl;
	    queue->context()->spewInfo ();
	    fflush (stdout);
	}
    } else {
	printf ("(Not displaying more than 20 invalid solutions)" << endl;
    }
    fflush (stdout);
}


static bool
trial_upgrade_cb (PoolItem_Ref original, PoolItem_Ref upgrade, void *user_data)
{
    Resolver *resolver = (Resolver *)user_data;

    resolver->addPoolItemToInstall (upgrade);

    RESULT << "Upgrading " << original->asString() << " => " << upgrade->asString() << endl;

    return false;
}


static void
print_marked_cb (PoolItem_Ref poolItem, void *data)
{
    RESULT << poolItem << " " << item.status() << endl;
    return;
}


static void
freshen_marked_cb (PoolItem_Ref poolItem, void *data)
{
    Resolver *resolver = (Resolver *)data;
    if (poolItem.status().isIncomplete()) {
	resolver->addPoolItemToInstall (poolItem);
    }

    return;
}


static void
parse_xml_trial (XmlNode_Ptr node)
{
    bool verify = false;
    bool instorder = false;

    assertExit (node->equals ("trial"));

    if (getenv ("RC_SPEW_XML")) cerr << "parse_xml_setup()" << endl;

    if (! done_setup) {
	cerr << "Any trials must be preceeded by the setup!" << endl;
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
		cerr << "Unknown channel '%s' (current)\n", channel_name.c_str());
	    }

	} else if (node->equals ("subscribe")) {

	    string channel_name = node->getProp ("channel");
	    Channel_Ptr channel = get_channel (channel_name);

	    if (channel != NULL) {
		channel->setSubscription (true);
	    } else {
		cerr << "Unknown channel '%s' (subscribe)\n", channel_name.c_str());
	    }

	} else if (node->equals ("install")) {

	    string channel_name = node->getProp ("channel");
	    string package_name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");

	    PoolItem_Ref poolItem;

	    assertExit (!channel_name.empty());
	    assertExit (!package_name.empty());

	    poolItem = get_poolItem (channel_name, package_name, kind_name);
	    if (poolItem) {
		RESULT << "Installing " << package_name << " from channel " << channel_name << endl;;
		resolver.addPoolItemToInstall (poolItem);
	    } else {
		cerr << "Unknown package %s::%s\n", channel_name.c_str(), package_name.c_str());
	    }

	} else if (node->equals ("uninstall")) {

	    string package_name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");

	    PoolItem_Ref poolItem;

	    assertExit (!package_name.empty());

	    poolItem = get_poolItem ("@system", package_name, kind_name);
	    if (poolItem) {
		RESULT << "Uninstalling " << package_name << endl;
		resolver.addPoolItemToRemove (poolItem);
	    } else {
		cerr << "Unknown system package %s\n", package_name.c_str());
	    }

	} else if (node->equals ("upgrade")) {
	    int count;

	    RESULT << "Checking for upgrades..." << endl;

	    count = world->foreachSystemUpgrade (true, trial_upgrade_cb, (void *)&resolver);
	    
	    if (count == 0)
		RESULT << "System is up-to-date, no upgrades required" << endl;
	    else
		RESULT << "Upgrading " << count << " package" << (count > 1 ? "s" : "") << endl;

	} else if (node->equals ("establish")
		   || node->equals ("freshen")) {

	    RESULT << "Establishing state ..." << endl;

	    resolver.establishState (established);
//cerr << "established<" << established << "> -> <" << resolver.bestContext() << ">" << endl;
	    established = resolver.bestContext();
	    if (established == NULL)
		RESULT << "Established NO context !" << endl;
	    else {
		RESULT << "Established context" << endl;
		established->foreachMarked (print_marked_cb, NULL);
		if (node->equals ("freshen")) {
		    RESULT << "Freshening ..." << endl;
		    established->foreachMarked (freshen_marked_cb, &resolver);
		}
	    }

	} else if (node->equals ("instorder")) {

	    RESULT << "Calculating installation order ..." << endl;

	    instorder = true;

	} else if (node->equals ("solvedeps")) {
#if 0
	    XmlNode_Ptr iter = node->children();

	    while (iter != NULL) {
		Dependency_Ptr dep = new Dependency (iter);

		/* We just skip over anything that doesn't look like a dependency. */

		if (dep) {
		    string conflict_str = iter->getProp ("conflict");

		    RESULT << "Solvedeps " << (conflict_str.empty() ? "" : "conflict ") << dep->asString().c_str() << endl;

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

	    PoolItemSet poolItems;

	    printf ("poolItems providing '%s'\n", prov_name.c_str());

	    poolItems = get_providing_poolItems (prov_name, kind_name);

	    if (poolItems.empty()) {
		cerr << "None found" << endl;
	    } else {
		for (PoolItemSet::const_iterator iter = poolItems.begin(); iter != poolItems.end(); ++iter) {
		    printf ("%s\n", (*iter)->asString().c_str());
		}
	    }

	} else if (node->equals ("whatdependson")) {

	    string channel_name = node->getProp ("channel");
	    string package_name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");
	    string prov_name = node->getProp ("provides");

	    PoolItemSet poolItems;

	    assert (!channel_name.empty());
	    assert (!package_name.empty());

	    if (!prov_name.empty()) {
		if (!package_name.empty()) {
		    cerr << "<whatdependson ...> can't have both package and provides." << endl;
		    exit (1);
		}
		poolItems = get_providing_poolItems (prov_name, kind_name);
	    }
	    else {
		PoolItem_Ref poolItem = get_poolItem (channel_name, package_name, kind_name);
		if (poolItem) poolItems.insert (poolItem);
	    }
	    if (poolItems.empty()) {
		cerr << "Can't find matching package" << endl;
	    } else {
		for (PoolItemSet::const_iterator iter = poolItems.begin(); iter != poolItems.end(); ++iter) {
		    PoolItemSet dependants = whatdependson (*iter);
		    for (PoolItemSet::const_iterator dep_iter = dependants.begin(); dep_iter != dependants.end(); ++dep_iter) {
			printf ("%s\n", (*dep_iter)->asString().c_str());
		    }
		}
	    }

	} else if (node->equals ("reportproblems")) {
	    if (resolver.resolveDependencies (established) == true) {
		RESULT << "No problems so far" << endl;
	    }
	    else {
		ResolverProblemList problems = resolver.problems ();
		RESULT << problems.size() << " problems found:" << endl;
		cout << ResolverProblem::toString(problems) << endl;
	    }

	} else {
	    cerr << "Unknown tag '%s' in trial\n", node->name().c_str());
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
		cerr << "Unknown tag '%s' in test\n", node->name().c_str());
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
	cerr << "Can't parse test file '%s'\n", filename.c_str());
	exit (0);
    }

    root = new XmlNode (xmlDocGetRootElement (xml_doc));

    if (getenv ("RC_SPEW_XML")) cerr << "Parsing file '%s'\n", filename.c_str());
    
    parse_xml_test (root);
    
    xmlFreeDoc (xml_doc);
}


//---------------------------------------------------------------------------------------------------------------------
#endif


static void
init_libzypp (void)
{
//    rc_version_set_global (rc_version_rpm_new());		//  rpm is the default for GVersion

//      World::setGlobalWorld (new MultiWorld());
}

int
main (int argc, char *argv[])
{
    setenv("ZYPP_NOLOG","1",1); // no logging
    
    if (argc != 2) {
	cerr << "Usage: deptestomatic testfile.xml" << endl;
	exit (0);
    }

    init_libzypp ();

    globalPath = argv[1];
    globalPath = globalPath.substr (0, globalPath.find_last_of ("/") +1);

    if (getenv ("RC_SPEW_XML")) cerr << "init_libzypp() done" << endl;

//    process_xml_test_file (string (argv[1]));

    return 0;
}

