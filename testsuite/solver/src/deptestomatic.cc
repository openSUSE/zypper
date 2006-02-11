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

#include <sstream>
#include <iostream>
#include <map>
#include <set>

#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include "helix/XmlNode.h"

#include "zypp/Resolvable.h"
#include "zypp/ResTraits.h"
#include "zypp/ResPool.h"
#include "zypp/PoolItem.h"
#include "zypp/Capability.h"
#include "zypp/CapSet.h"
#include "zypp/CapFactory.h"
#include "zypp/ResolverProblem.h"
#include "zypp/ProblemSolution.h"


#include "zypp/Source.h"
#include "zypp/SourceFactory.h"
#include "zypp/SourceManager.h"
#include "zypp/source/SourceImpl.h"

#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"

#include "zypp/media/MediaManager.h"

#include "helix/HelixSourceImpl.h"

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"

#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/ResolverContext.h"
#include "zypp/solver/detail/ResolverQueue.h"
#include "zypp/solver/detail/ResolverInfo.h"
#include "zypp/ResolverProblem.h"
#include "zypp/solver/detail/InstallOrder.h"


using namespace std;
using namespace zypp;
using zypp::solver::detail::XmlNode;
using zypp::solver::detail::XmlNode_Ptr;
using zypp::solver::detail::ResolverContext_Ptr;
using zypp::solver::detail::ResolverQueueList;
using zypp::solver::detail::ResolverQueue_Ptr;
using zypp::solver::detail::InstallOrder;
using zypp::ResolverProblemList;

static string globalPath;

static ZYpp::Ptr God;
static SourceManager_Ptr manager;
static bool forceResolve;

typedef list<unsigned int> ChecksumList;

#define RESULT cout << ">!> "

static std::ostream &
printRes ( std::ostream & str, ResObject::constPtr r )
{
    if (r->kind() != ResTraits<zypp::Package>::kind)
	str << r->kind() << ':';
    str  << r->name() << '-' << r->edition();
    if (r->arch() != "") {
	str << '.' << r->arch();
    }
    Source_Ref s = r->source();
    if (s) {
	string alias = s.alias();
	if (!alias.empty()
	    && alias != "@system")
	{
	    str << '[' << s.alias() << ']';
	}
//	str << '[' << s << ']';
    }
    return str;
}

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
	else if (str == "selection") {
	    kind = ResTraits<zypp::Selection>::kind;
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
assemble_install_cb (PoolItem_Ref poolItem, const ResStatus & status, void *data)
{
    StringList *slist = (StringList *)data;
    ostringstream s;
    s << str::form ("%-7s ", poolItem.status().staysInstalled() ? "|flag" : "install");
    printRes (s, poolItem.resolvable());

    slist->push_back (s.str());
}


static void
assemble_uninstall_cb (PoolItem_Ref poolItem, const ResStatus & status, void *data)
{
    StringList *slist = (StringList *)data;
    ostringstream s;
MIL << "assemble_uninstall_cb(" << poolItem << "):" << status << endl;
    s << str::form ("%-7s ", poolItem.status().isImpossible () ? "|unflag" : "remove");
    printRes (s, poolItem.resolvable());

    slist->push_back (s.str());
}


static void
assemble_impossible_cb (PoolItem_Ref poolItem, const ResStatus & status, void *data)
{
    StringList *slist = (StringList *)data;
    ostringstream s;
MIL << "assemble_impossible_cb(" << poolItem << "):" << status << endl;
    s << str::form ("%-7s ", "|unflag");
    printRes (s, poolItem.resolvable());

    slist->push_back (s.str());
}


static void
assemble_upgrade_cb (PoolItem_Ref res1, const ResStatus & status, PoolItem_Ref res2, const ResStatus & status2, void *data)
{
    StringList *slist = (StringList *)data;
    ostringstream s;

    s << "upgrade ";

    printRes (s, res2.resolvable());
    s << " => ";
    printRes (s, res1.resolvable());

    slist->push_back (s.str());
}


static void
assemble_incomplete_cb (PoolItem_Ref poolItem, const ResStatus & status,void *data)
{
    StringList *slist = (StringList *)data;
    ostringstream s;
    s << str::form ("%-11s ", poolItem.status().wasInstalled() ? "incomplete" : "|needed");
    printRes (s, poolItem.resolvable());

    slist->push_back (s.str());
}


static void
assemble_satisfy_cb (PoolItem_Ref poolItem, const ResStatus & status, void *data)
{
    StringList *slist = (StringList *)data;
    ostringstream s;
    s << str::form ("%-10s ", poolItem.status().wasInstalled() ? "complete" : "|satisfied");
    printRes (s, poolItem.resolvable());

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

	context->foreachImpossible (assemble_impossible_cb, &items);

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

	InstallOrder order( context->pool(), installs, dummy );		 // sort according top prereq
	order.init();
	const PoolItemList & installorder ( order.getTopSorted() );
	for (PoolItemList::const_iterator iter = installorder.begin(); iter != installorder.end(); iter++) {
		RESULT; printRes (cout, (*iter)); cout << endl;
	}
	cout << "- - - - - - - - - -" << endl;
    }

    cout.flush();

    cout << "Context Info:" << endl;
    context->spewInfo ();

    cout << "Context Context:" << endl;
    cout << *context << endl;

    return;
}

//---------------------------------------------------------------------------------------------------------------------
struct FindPackage : public resfilter::ResObjectFilterFunctor
{
    PoolItem_Ref poolItem;
    Source_Ref source;
    Resolvable::Kind kind;

    FindPackage (Source_Ref s, Resolvable::Kind k)
	: source (s)
	, kind (k)
    { }

    bool operator()( PoolItem_Ref p)
    {
MIL << p << " ?" << endl;
	Source_Ref s = p->source();

	if (s.alias() != source.alias()) {
	    return true;
	}
	poolItem = p;
	return false;				// stop here, we found it
    }
};



static PoolItem_Ref
get_poolItem (const string & source_alias, const string & package_name, const string & kind_name = "")
{
    PoolItem_Ref poolItem;
    Resolvable::Kind kind = string2kind (kind_name);
    Source_Ref source;

    try {
	source = manager->findSource (source_alias);
    }
    catch (Exception & excpt_r) {
	ZYPP_CAUGHT (excpt_r);
	cerr << "Can't find source '" << source_alias << "'" << endl;
	return poolItem;
    }

    try {
	FindPackage info (source, kind);

	invokeOnEach( God->pool().byNameBegin( package_name ),
		      God->pool().byNameEnd( package_name ),
		      functor::chain( resfilter::BySource(source), resfilter::ByKind (kind) ),
//		      resfilter::ByKind (kind),
		      functor::functorRef<bool,PoolItem> (info) );

	poolItem = info.poolItem;
    }
    catch (Exception & excpt_r) {
	ZYPP_CAUGHT (excpt_r);
	cerr << "Can't find resolvable '" << package_name << "': source '" << source_alias << "' not defined" << endl;
	return poolItem;
    }

    if (!poolItem) {
	cerr << "Can't find resolvable '" << package_name << "' in source '" << source_alias << "': no such name/kind" << endl;
    }

    return poolItem;
}


//---------------------------------------------------------------------------------------------------------------------
// whatdependson


struct RequiringPoolItem : public resfilter::OnCapMatchCallbackFunctor
{
    PoolItemSet itemset;
    PoolItem_Ref provider;
    Capability cap;
    bool first;

    RequiringPoolItem (PoolItem_Ref p)
	: provider (p)
    { }

    bool operator()( PoolItem_Ref requirer, const Capability & match )
    {
	if (itemset.insert (requirer).second) {
	    if (first) {
		cout << "\t" << provider.resolvable() << " provides " << cap << " required by" << endl;
		first = false;
	    }
	    cout << "\t\t" << requirer.resolvable() << " for " << cap << endl;
	}
	return true;
    }
};


static PoolItemSet
whatdependson (PoolItem_Ref poolItem)
{
    cout << endl << endl << "What depends on '" << poolItem.resolvable() << "'" << endl;

    RequiringPoolItem info (poolItem);

    // loop over all provides and call foreachRequiringResItem

    CapSet caps = poolItem->dep (Dep::PROVIDES);
    for (CapSet::const_iterator cap_iter = caps.begin(); cap_iter != caps.end(); ++cap_iter) {

	info.cap = *cap_iter;
	info.first = true;

	//world->foreachRequiringResItem (info.cap, requires_poolItem_cb, &info);

	Dep dep( Dep::REQUIRES );
	invokeOnEach( God->pool().byCapabilityIndexBegin( info.cap.index(), dep ),
		      God->pool().byCapabilityIndexEnd( info.cap.index(), dep ),
		      resfilter::callOnCapMatchIn( dep, info.cap, functor::functorRef<bool,PoolItem,Capability>(info) ) );

    }

    return info.itemset;
}


//---------------------------------------------------------------------------------------------------------------------
// whatprovides


struct ProvidingPoolItem : public resfilter::OnCapMatchCallbackFunctor
{
    PoolItemSet itemset;

    bool operator()( PoolItem_Ref provider, const Capability & match )
    {
	itemset.insert (provider);
	return true;
    }
};


static PoolItemSet
get_providing_poolItems (const string & prov_name, const string & kind_name = "")
{
    PoolItemSet rs;
    Resolvable::Kind kind = string2kind (kind_name);

    CapFactory factory;
    Capability cap = factory.parse (kind, prov_name);

    Dep dep( Dep::PROVIDES );
    ProvidingPoolItem info;

    // world->foreachProvidingResItem (cap, providing_poolItem_cb, &rs);

    invokeOnEach( God->pool().byCapabilityIndexBegin( cap.index(), dep ),
		  God->pool().byCapabilityIndexEnd( cap.index(), dep ),
		  resfilter::callOnCapMatchIn( dep, cap, functor::functorRef<bool,PoolItem,Capability>(info) ) );

    return info.itemset;
}



//---------------------------------------------------------------------------------------------------------------------
// setup related functions

typedef multimap<string,PoolItem_Ref> ItemMap;

struct SortItem : public resfilter::PoolItemFilterFunctor
{
    ItemMap sorted;

    SortItem()
    { }

    bool operator()( PoolItem_Ref poolItem )
    {
	ostringstream ostr;
	printRes (ostr, poolItem);
	sorted.insert (ItemMap::value_type(ostr.str(), poolItem));
	return true;
    }
};


// collect all installed items in a set

void
print_pool (const string & prefix = "")
{
    SortItem info;
    cout << "Current pool:" << endl;
    invokeOnEach( God->pool().begin( ),
		  God->pool().end ( ),
		  functor::functorRef<bool,PoolItem> (info) );

    int count = 0;
    for (ItemMap::const_iterator it = info.sorted.begin(); it != info.sorted.end(); ++it) {
	cout << prefix << ++count << ": ";
	cout << it->second;
	cout << endl;
    }
    return;
}


static void
load_source (const string & alias, const string & filename, const string & type, bool system_packages)
{
    Pathname pathname = globalPath + filename;
    int count = 0;

    try {
	Url url("file://");

	media::MediaManager mmgr;
	media::MediaId mediaid = mmgr.open(url);
	HelixSourceImpl *impl = new HelixSourceImpl ();
	impl->factoryCtor (mediaid, pathname, alias);
        Source_Ref src( SourceFactory().createFrom(impl) );

	unsigned snum = manager->addSource (src);

	count = src.resolvables().size();
	cout << "Added source '" << alias << "' as #" << snum  << ":[" << src.alias() << "] with " << count << " resolvables" << endl;
	God->addResolvables( src.resolvables(), (alias == "@system") );
//	print_pool ();

	cout << "Loaded " << count << " package(s) from " << pathname << endl;
    }
    catch ( Exception & excpt_r ) {
	ZYPP_CAUGHT (excpt_r);
	cout << "Loaded NO package(s) from " << pathname << endl;
    }
}

static void
undump (const std::string & filename)
{
    cerr << "undump not really supported" << endl;

    return load_source ("undump", filename, "undump", false);
}


static bool done_setup = false;

static void
parse_xml_setup (XmlNode_Ptr node)
{
    if (!node->equals("setup")) {
	ZYPP_THROW (Exception ("Node not 'setup' in parse_xml_setup():"+node->name()));
    }

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
	if (node->equals ("forceResolve")) {
            
            forceResolve = true;
        } else if (node->equals ("system")) {

	    string file = node->getProp ("file");
	    load_source ("@system", file, "helix", true);

	} else if (node->equals ("channel")) {

	    string name = node->getProp ("name");
	    string file = node->getProp ("file");
	    string type = node->getProp ("type");
	    load_source (name, file, type, false);

	} else if (node->equals ("undump")) {

	    string file = node->getProp ("file");
	    undump (file);

	} else if (node->equals ("force-install")) {

	    string source_alias = node->getProp ("channel");
	    string package_name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");

	    PoolItem_Ref poolItem;

	    poolItem = get_poolItem (source_alias, package_name, kind_name);
	    if (poolItem) {
		RESULT << "Force-installing " << package_name << " from channel " << source_alias << endl;;

		poolItem.status().setStatus(ResStatus::installed);

#if 0
		Source_Ref system_source = manager->findSource("@system");

		if (!system_source)
		    cerr << "No system source available!" << endl;
		PoolItem_Ref r = boost::const_pointer_cast<PoolItem>(poolItem);
		r->setChannel (system_source);
#endif
	    } else {
		cerr << "Unknown package " << source_alias << "::" << package_name << endl;
	    }

	} else if (node->equals ("force-uninstall")) {

	    string package_name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");

	    PoolItem_Ref poolItem;

	    poolItem = get_poolItem ("@system", package_name, kind_name);
	    
	    if (! poolItem) {
		cerr << "Can't force-uninstall installed package '" << package_name << "'" << endl;
	    } else {
		RESULT << "Force-uninstalling " << package_name << endl;
		poolItem.status().setStatus(ResStatus::uninstalled);
#if 0
		God->pool().remove (poolItem);
#endif
	    }

	} else if (node->equals ("lock")) {

	    string source_alias = node->getProp ("channel");
	    string package_name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");

	    PoolItem_Ref poolItem;

	    poolItem = get_poolItem (source_alias, package_name, kind_name);
	    if (poolItem) {
		RESULT << "Locking " << package_name << " from channel " << source_alias << endl;
#warning Needs locks
#if 0
		r->setLocked (true);
#endif
	    } else {
		cerr << "Unknown package " << source_alias << "::" << package_name << endl;
	    }

	} else {
	    cerr << "Unrecognized tag '" << node->name() << "' in setup" << endl;
	}

	node = node->next();
    }
}

//---------------------------------------------------------------------------------------------------------------------
// trial related functions

static void
report_solutions ( solver::detail::Resolver_Ptr resolver, bool instorder)
{
    int count = 1;
    ChecksumList checksum_list;

    cout << endl;

    if (!resolver->completeQueues().empty()) {
	cout << "Completed solutions: " << (long) resolver->completeQueues().size() << endl;
    }

    if (resolver->prunedQueues().empty()) {
	cout << "Pruned solutions: " << (long) resolver->prunedQueues().size() << endl;
    }

    if (resolver->deferredQueues().empty()) {
	cout << "Deferred solutions: " << (long) resolver->deferredQueues().size() << endl;
    }

    if (resolver->invalidQueues().empty()) {
	cout << "Invalid solutions: " << (long) resolver->invalidQueues().size() << endl;
    }
    
    if (resolver->bestContext()) {
	cout << endl << "Best Solution:" << endl;
	print_solution (resolver->bestContext(), &count, checksum_list, instorder);

	ResolverQueueList complete = resolver->completeQueues();
	if (complete.size() > 1)
	    cout << endl << "Other Valid Solutions:" << endl;

	if (complete.size() < 20) {
	    for (ResolverQueueList::const_iterator iter = complete.begin(); iter != complete.end(); iter++) {
		ResolverQueue_Ptr queue = (*iter);
		if (queue->context() != resolver->bestContext()) 
		    print_solution (queue->context(), &count, checksum_list, instorder);
	    }
	}
    }

    ResolverQueueList invalid = resolver->invalidQueues();
    if (invalid.size() < 20) {
	cout << endl;

	for (ResolverQueueList::const_iterator iter = invalid.begin(); iter != invalid.end(); iter++) {
	    ResolverQueue_Ptr queue = (*iter);
	    cout << "Failed Solution: " << endl << *queue->context() << endl;
	    cout << "- - - - - - - - - -" << endl;
	    queue->context()->spewInfo ();
	    fflush (stdout);
	}
    } else {
	cout << "(Not displaying more than 20 invalid solutions)" << endl;
    }
    fflush (stdout);
}

//-----------------------------------------------------------------------------
// system Upgrade

struct Unique : public resfilter::PoolItemFilterFunctor
{
    PoolItemSet itemset;

    bool operator()( PoolItem_Ref poolItem )
    {
	itemset.insert (poolItem);
	return true;
    }
};


// collect all installed items in a set

PoolItemSet
uniquelyInstalled (void)
{
    Unique info;

    invokeOnEach( God->pool().begin( ),
		  God->pool().end ( ),
		  resfilter::ByInstalled (),
		  functor::functorRef<bool,PoolItem> (info) );
    return info.itemset;
}


// keep upgrades in a map to achieve lexically sorted debug output

typedef pair<PoolItem_Ref,PoolItem_Ref> UpgradePair;
typedef map<string,UpgradePair > UpgradeMap;

struct DoUpgrades : public resfilter::OnCapMatchCallbackFunctor, public resfilter::PoolItemFilterFunctor
{
    PoolItem_Ref installed;
    UpgradeMap upgrades;
//    PoolItemSet upgrades;
    solver::detail::Resolver_Ptr resolver;
    int count;

    DoUpgrades (solver::detail::Resolver_Ptr r)
	: resolver (r)
	, count (0)
    {  }

    bool operator()( PoolItem_Ref poolItem )
    {
	if (installed->edition().compare (poolItem->edition()) < 0) {

	UpgradeMap::const_iterator pos = upgrades.find(poolItem->name());
	if (pos == upgrades.end()) {
	    upgrades[poolItem->name()] = make_pair (installed, poolItem);
	    ++count;
	    return false;
	}
	}
#if 0		// disabled in favor of lexical ordering
	if (upgrades.insert (poolItem).second) {			// only consider first match
	    resolver->addPoolItemToInstall (poolItem);
	    RESULT << "Upgrading ";
	    printRes (cout, installed);
	    cout << " => ";
	    printRes (cout, poolItem);
	    cout << endl;
	    ++count;
	}
#endif
	return true;
    }
};


int
foreach_system_upgrade (solver::detail::Resolver_Ptr resolver)
{
    PoolItemSet installed = uniquelyInstalled();
    DoUpgrades info (resolver);

    // world->foreachSystemUpgrade (true, trial_upgrade_cb, (void *)&resolver);

    for (PoolItemSet::iterator iter = installed.begin(); iter != installed.end(); ++iter) {
	PoolItem_Ref p = *iter;
	info.installed = p;
	invokeOnEach( God->pool().byNameBegin( p->name() ), God->pool().byNameEnd( p->name() ),
			functor::chain( resfilter::ByUninstalled(), resfilter::ByKind( p->kind() ) ),
#if 0
		      functor::chain( resfilter::ByUninstalled(),
			functor::chain( resfilter::ByKind( p->kind() ),
					resfilter::byEdition<CompareByGT<Edition> >( p->edition() ) ) ),
#endif
		      functor::functorRef<bool,PoolItem>(info) );

    }

    // now output in alphabetical order

    for (UpgradeMap::const_iterator iter = info.upgrades.begin(); iter != info.upgrades.end(); ++iter) {
	UpgradePair i_and_u = iter->second;
	resolver->addPoolItemToInstall (i_and_u.second);
	RESULT << "Upgrading ";
	printRes (cout, i_and_u.first);
	cout << " => ";
	printRes (cout, i_and_u.second);
	cout << endl;
    }

    return info.count;
}

//-----------------------------------------------------------------------------
// ResolverContext output

static void
print_marked_cb (PoolItem_Ref poolItem, const ResStatus & status, void *data)
{
    RESULT; printRes (cout, poolItem.resolvable()); cout << " " << status << endl;
    return;
}


static void
freshen_marked_cb (PoolItem_Ref poolItem, const ResStatus & status, void *data)
{
    solver::detail::Resolver_Ptr resolver = *((solver::detail::Resolver_Ptr *)data);
    if (status.isNeeded()) {
	resolver->addPoolItemToInstall (poolItem);
    }

    return;
}


static void
parse_xml_trial (XmlNode_Ptr node, const ResPool & pool)
{
    bool verify = false;
    bool instorder = false;
    bool distupgrade = false;

    if (!node->equals ("trial")) {
	ZYPP_THROW (Exception ("Node not 'trial' in parse_xml_trial()"));
    }

    DBG << "parse_xml_trial()" << endl;

    if (! done_setup) {
	cerr << "Any trials must be preceeded by the setup!" << endl;
	exit (0);
    }

    print_sep ();

    solver::detail::Resolver_Ptr resolver = new solver::detail::Resolver (pool);

    resolver->setForceResolve (forceResolve);

    ResolverContext_Ptr established = NULL;

    node = node->children();
    while (node) {
	if (!node->isElement()) {
	    node = node->next();
	    continue;
	}

	if (node->equals("note")) {

	    string note = node->getContent ();
	    cout << "NOTE: " << note << endl;

	} else if (node->equals ("verify")) {

	    verify = true;

	} else if (node->equals ("current")) {

	    string source_alias = node->getProp ("channel");
	    Source_Ref source = manager->findSource (source_alias);

	    if (source) {
//FIXME		resolver->setCurrentChannel (source);
	    } else {
		cerr << "Unknown source '" << source_alias << "' (current)" << endl;
	    }

	} else if (node->equals ("subscribe")) {

	    string source_alias = node->getProp ("channel");
	    Source_Ref source = manager->findSource (source_alias);

	    if (source) {
//FIXME		source->setSubscription (true);
	    } else {
		cerr << "Unknown source '" << source_alias << "' (subscribe)" << endl;
	    }

	} else if (node->equals ("install")) {

	    string source_alias = node->getProp ("channel");
	    string name = node->getProp ("name");
	    if (name.empty())
		name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");
	    string soft = node->getProp ("soft");

	    PoolItem_Ref poolItem;

	    poolItem = get_poolItem (source_alias, name, kind_name);
	    if (poolItem) {
		RESULT << "Installing " << name << " from channel " << source_alias << endl;;
		poolItem.status().setToBeInstalled(ResStatus::USER);
		if (!soft.empty())
		    poolItem.status().setSoftInstall(true);
//		resolver->addPoolItemToInstall (poolItem);
	    } else {
		cerr << "Unknown item " << source_alias << "::" << name << endl;
	    }

	} else if (node->equals ("uninstall")) {

	    string name = node->getProp ("name");
	    if (name.empty())
		name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");
	    string soft = node->getProp ("soft");

	    PoolItem_Ref poolItem;

	    poolItem = get_poolItem ("@system", name, kind_name);
	    if (poolItem) {
		RESULT << "Uninstalling " << name << endl;
		poolItem.status().setToBeUninstalled(ResStatus::USER);
		if (!soft.empty())
		    poolItem.status().setSoftUninstall(true);
//		resolver->addPoolItemToRemove (poolItem);
	    } else {
		cerr << "Unknown system item " << name << endl;
	    }

	} else if (node->equals ("upgrade")) {

	    RESULT << "Checking for upgrades..." << endl;

	    int count = foreach_system_upgrade (resolver);
	    
	    if (count == 0)
		RESULT << "System is up-to-date, no upgrades required" << endl;
	    else
		RESULT << "Upgrading " << count << " package" << (count > 1 ? "s" : "") << endl;

	} else if (node->equals ("distupgrade")) {

	    distupgrade = true;

	    RESULT << "Doing distribution upgrade ..." << endl;
	    UpgradeStatistics stats;

	    string delete_unmaintained = node->getProp ("delete_unmaintained");
	    if (delete_unmaintained == "false") {
		stats.delete_unmaintained = false;
	    }

	    resolver->doUpgrade(stats);

	    print_pool(">!>");

	} else if (node->equals ("establish")
		   || node->equals ("freshen")) {

	    RESULT << "Establishing state ..." << endl;

	    resolver->establishState (established);
//cerr << "established<" << established << "> -> <" << resolver->bestContext() << ">" << endl;
	    established = resolver->bestContext();
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

		    resolver->addExtraDependency (dep);

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

	    cout << "poolItems providing '" << prov_name << "'" << endl;

	    poolItems = get_providing_poolItems (prov_name, kind_name);

	    if (poolItems.empty()) {
		cerr << "None found" << endl;
	    } else {
		for (PoolItemSet::const_iterator iter = poolItems.begin(); iter != poolItems.end(); ++iter) {
		    cout << (*iter) << endl;
		}
	    }

	} else if (node->equals ("whatdependson")) {

	    string source_alias = node->getProp ("channel");
	    string package_name = node->getProp ("package");
	    string kind_name = node->getProp ("kind");
	    string prov_name = node->getProp ("provides");

	    PoolItemSet poolItems;

	    assert (!source_alias.empty());
	    assert (!package_name.empty());

	    if (!prov_name.empty()) {
		if (!package_name.empty()) {
		    cerr << "<whatdependson ...> can't have both package and provides." << endl;
		    exit (1);
		}
		poolItems = get_providing_poolItems (prov_name, kind_name);
	    }
	    else {
		PoolItem_Ref poolItem = get_poolItem (source_alias, package_name, kind_name);
		if (poolItem) poolItems.insert (poolItem);
	    }
	    if (poolItems.empty()) {
		cerr << "Can't find matching package" << endl;
	    } else {
		for (PoolItemSet::const_iterator iter = poolItems.begin(); iter != poolItems.end(); ++iter) {
		    PoolItemSet dependants = whatdependson (*iter);
		    for (PoolItemSet::const_iterator dep_iter = dependants.begin(); dep_iter != dependants.end(); ++dep_iter) {
			cout << (*dep_iter) << endl;
		    }
		}
	    }

	} else if (node->equals ("reportproblems")) {
	    if (resolver->resolvePool() == true) {
		RESULT << "No problems so far" << endl;
	    }
	    else {
		ResolverProblemList problems = resolver->problems ();
		RESULT << problems.size() << " problems found:" << endl;
		for (ResolverProblemList::iterator iter = problems.begin(); iter != problems.end(); ++iter) {
                    ResolverProblem problem = **iter;
                    RESULT << "Problem:" << endl;
                    RESULT << problem.description() << endl;
                    RESULT << problem.details() << endl;
                    
                    ProblemSolutionList solutions = problem.solutions();
                    for (ProblemSolutionList::const_iterator iter = solutions.begin();
                         iter != solutions.end(); ++iter) {
                        ProblemSolution solution = **iter;
                        RESULT << "   Solution:" << endl;
                        RESULT << "      " << solution.description() << endl;
                        RESULT << "      " << solution.details() << endl;
                    }
		}
	    }
	} else if (node->equals ("takesolution")) {
	    string problemNrStr = node->getProp ("problem");
	    string solutionNrStr = node->getProp ("solution");
	    assert (!problemNrStr.empty());
	    assert (!solutionNrStr.empty());
            int problemNr = atoi (problemNrStr.c_str());
            int solutionNr = atoi (solutionNrStr.c_str());
            RESULT << "Taking solution: " << solutionNr << endl;
            RESULT << "For problem:     " << problemNr << endl;
            ResolverProblemList problems = resolver->problems ();
            
            int problemCounter = 0;
            int solutionCounter = 0;
            // find problem
            for (ResolverProblemList::iterator probIter = problems.begin();
                 probIter != problems.end(); ++probIter) {
                if (problemCounter == problemNr) {
                    ResolverProblem problem = **probIter;
                    ProblemSolutionList solutionList = problem.solutions();
                    //find solution
                    for (ProblemSolutionList::iterator solIter = solutionList.begin();
                         solIter != solutionList.end(); ++solIter) {
                        if (solutionCounter == solutionNr) {
                            ProblemSolution_Ptr solution = *solIter;
                            cout << "Taking solution: " << endl << *solution << endl;
                            cout << "For problem: " << endl << problem << endl;
                            ProblemSolutionList doList;
                            doList.push_back (solution);
                            resolver->applySolutions (doList);
                            break;
                        }
                        solutionCounter++;
                    }
                    break;
                }
                problemCounter++;
            }

            if (problemCounter != problemNr) {
                RESULT << "Wrong problem number (0-" << problemCounter-1 << ")" << endl;
            } else if (solutionCounter != solutionNr) {
                RESULT << "Wrong solution number (0-" << solutionCounter-1 << ")" <<endl;
            } else {
                // resolve and check it again
                if (resolver->resolvePool() == true) {
                    RESULT << "No problems so far" << endl;
                }
                else {
                    ResolverProblemList problems = resolver->problems ();
                    RESULT << problems.size() << " problems found:" << endl;
                    for (ResolverProblemList::iterator iter = problems.begin(); iter != problems.end(); ++iter) {
                        cout << **iter << endl;
                    }
                }
            }
	} else {
	    cerr << "Unknown tag '" << node->name() << "' in trial" << endl;
	}

	node = node->next();
    }

    if (getenv ("RC_DEPS_TIME")) {
	int timeout = atoi (getenv ("RC_DEPS_TIME"));

	resolver->setTimeout (timeout);
    }

    if (verify)
	resolver->verifySystem ();
#if 0
    else if (distupgrade)
	resolver->resolvePool();
    else
	resolver->resolveDependencies (established);
#else
    else
	resolver->resolvePool();

#endif

    report_solutions (resolver, instorder);
}

//---------------------------------------------------------------------------------------------------------------------

static void
parse_xml_test (XmlNode_Ptr node, const ResPool & pool)
{
    if (!node->equals("test")) {
	ZYPP_THROW (Exception("Node not 'test' in parse_xml_test():"+node->name()));
    }

    node = node->children();

    while (node) {
	if (node->type() == XML_ELEMENT_NODE) {
	    if (node->equals("setup")) {
		parse_xml_setup (node);
	    } else if (node->equals ("trial")) {
		parse_xml_trial (node, pool);
	    } else {
		cerr << "Unknown tag '" << node->name() << "' in test" << endl;
	    }
	}

	node = node->next();
    }
}


static void
process_xml_test_file (const string & filename, const ResPool & pool)
{
    xmlDocPtr xml_doc;
    XmlNode_Ptr root;

    xml_doc = xmlParseFile (filename.c_str());
    if (xml_doc == NULL) {
	cerr << "Can't parse test file '" << filename << "'" << endl;
	exit (0);
    }

    root = new XmlNode (xmlDocGetRootElement (xml_doc));

    DBG << "Parsing file '" << filename << "'" << endl;
    
    parse_xml_test (root, pool);
    
    xmlFreeDoc (xml_doc);
}


//---------------------------------------------------------------------------------------------------------------------

int
main (int argc, char *argv[])
{
//    setenv("ZYPP_NOLOG","1",1); // no logging
    
    if (argc != 2) {
	cerr << "Usage: deptestomatic testfile.xml" << endl;
	exit (0);
    }
    forceResolve = false;
    manager = SourceManager::sourceManager();
    ZYppFactory zf;
    God = zf.getZYpp();

    God->resolver()->setArchitecture(Arch(""));		// override ZYpp

    globalPath = argv[1];
    globalPath = globalPath.substr (0, globalPath.find_last_of ("/") +1);

    DBG << "init_libzypp() done" << endl;

    process_xml_test_file (string (argv[1]), God->pool());

    return 0;
}

