/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Resolver_problems.cc
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <map>
#include <sstream>

#include "zypp/solver/detail/Resolver.h"
#include "zypp/Resolver.h"
#include "zypp/solver/detail/ResolverContext.h"
#include "zypp/ResolverProblem.h"
#include "zypp/solver/detail/ProblemSolutionIgnore.h"
#include "zypp/solver/detail/ProblemSolutionInstall.h"
#include "zypp/solver/detail/ProblemSolutionUninstall.h"
#include "zypp/solver/detail/ProblemSolutionUnlock.h"
#include "zypp/solver/detail/ProblemSolutionKeep.h"

#include "zypp/solver/detail/ResolverInfoChildOf.h"
#include "zypp/solver/detail/ResolverInfoConflictsWith.h"
#include "zypp/solver/detail/ResolverInfoContainer.h"
#include "zypp/solver/detail/ResolverInfoDependsOn.h"
#include "zypp/solver/detail/ResolverInfoMisc.h"
#include "zypp/solver/detail/ResolverInfoMissingReq.h"
#include "zypp/solver/detail/ResolverInfoNeededBy.h"
#include "zypp/solver/detail/ResolverInfoObsoletes.h"

#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"

#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"


/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

using namespace std;

typedef map<PoolItem_Ref, ResolverInfo_Ptr> ProblemMap;
typedef multimap<PoolItem_Ref, Capability> ItemCapabilityMap;
typedef multimap<PoolItem_Ref, PoolItem_Ref> ConflictMap;	

// match template over ItemCapabilityMap
template <class K, class V>
class cap_equals {
  private:
    V value;
public:
    cap_equals (const V& v)
	: value(v) {
    }
    // comparison
    bool operator() (pair<const K, V> elem) {
	return value.matches (elem.second) == CapMatch::yes;
    }
};

// match template over ConflictMap
template <class K, class V>
class conflict_equals {
  private:
    V value;
public:
    conflict_equals (const V& v)
	: value(v) {
    }
    // comparison
    bool operator() (pair<const K, V> elem) {
	return value = elem.second;
    }
};	
	
// set resolvables with errors

typedef struct {
    ProblemMap problems;
    // A map of PoolItems which provides a capability but are set
    // for uninstallation
    ItemCapabilityMap provideAndDeleteMap;
    // A map of PoolItems which provides a capability but are locked
    ItemCapabilityMap provideAndLockMap;
    // A map of conflicting Items
    ConflictMap conflictMap;
} ResItemCollector;


static void
collector_cb (ResolverInfo_Ptr info, void *data)
{
    ResItemCollector *collector = (ResItemCollector *)data;
    PoolItem_Ref item = info->affected();
    if (item
	&& info->error()) {
	collector->problems[item] = info;
    }
    // Collicting items which are providing requirements but they
    // are set for uninstall
    if (info->type() == RESOLVER_INFO_TYPE_UNINSTALL_PROVIDER) {
	ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
	// does entry already exists ?
	ItemCapabilityMap::iterator pos = find_if (collector->provideAndDeleteMap.begin(),
						   collector->provideAndDeleteMap.end(),
						   cap_equals<PoolItem_Ref, Capability>(misc_info->capability()));
	
	if (pos == collector->provideAndDeleteMap.end()) {
	    _XDEBUG ("Inserting " << misc_info->capability() << "/" <<  misc_info->other()
		     << " into provideAndDelete map");
	    collector->provideAndDeleteMap.insert (make_pair( misc_info->other(), misc_info->capability()));
	}
    }
    // Collecting items which are providing requirements but they
    // are locked
    if (info->type() == RESOLVER_INFO_TYPE_LOCKED_PROVIDER) {
	ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
	// does entry already exists ?
	ItemCapabilityMap::iterator pos = find_if (collector->provideAndLockMap.begin(),
						   collector->provideAndLockMap.end(),
						   cap_equals<PoolItem_Ref, Capability>(misc_info->capability()));
	
	if (pos == collector->provideAndLockMap.end()) {
	    _XDEBUG ("Inserting " << misc_info->capability() << "/" <<  misc_info->other()
		     << " into provideAndLockMap map");
	    collector->provideAndLockMap.insert (make_pair( misc_info->other(), misc_info->capability()));
	}
    }

    // Collecting all conflicting Items
    if (info->type() == RESOLVER_INFO_TYPE_CONFLICT_UNINSTALLABLE
	|| info->type() == RESOLVER_INFO_TYPE_CONFLICT_CANT_INSTALL) {
	ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);

	// does entry already exists ?
	ConflictMap::iterator pos = find_if (collector->conflictMap.lower_bound(misc_info->other()),
					     collector->conflictMap.upper_bound(misc_info->other()),
					     conflict_equals<PoolItem_Ref, PoolItem_Ref>(misc_info->affected()));
	if (pos == collector->conflictMap.end()) {
	    _XDEBUG ("Inserting " << misc_info->affected() << "/" <<  misc_info->other()
		     << " into conflictMap map");
	    collector->conflictMap.insert (make_pair(misc_info->affected(), misc_info->other()));
	    collector->conflictMap.insert (make_pair(misc_info->other(), misc_info->affected())); // reverse
	}
    }
    
}

struct AllRequires
{
    PoolItemList requirers;

    bool operator()( const CapAndItem & cai )
    {
	DBG << cai.item << " requires " << cai.cap << endl;
	requirers.push_back( cai.item );

	return true;
    }
};


ResolverProblemList
Resolver::problems (void) const
{
    ResolverProblemList problems;

    if (_best_context) {
	MIL << "Valid solution found, no problems" << endl;
	return problems;
    }

    // collect all resolvables with error

    ResolverQueueList invalid = invalidQueues();
    MIL << invalid.size() << " invalid queues" << endl;

    if (invalid.empty()) {
	WAR << "No solver problems, but there is also no valid solution." << endl;
	return problems;
    }

    ResolverContext_Ptr context = invalid.front()->context();
    ResItemCollector collector;
    context->foreachInfo (PoolItem(), RESOLVER_INFO_PRIORITY_VERBOSE, collector_cb, &collector);

    for (ProblemMap::const_iterator iter = collector.problems.begin(); iter != collector.problems.end(); ++iter) {
	PoolItem_Ref item = iter->first;
	ResolverInfo_Ptr info = iter->second;

	bool problem_created = false;

	DBG << "Problem: " << *info;
	DBG << "; Evaluate solutions..." << endl;
	
	string who = item->name();
	string what;
	string details;
	switch (info->type()) {
	    case RESOLVER_INFO_TYPE_INVALID: {
		what = _("Invalid information");
	    }
	    break;
	    case RESOLVER_INFO_TYPE_NEEDED_BY: { // no solution; it is only a info
		ResolverInfoNeededBy_constPtr needed_by = dynamic_pointer_cast<const ResolverInfoNeededBy>(info);
		if (needed_by->items().size() >= 1)
		    // TranslatorExplanation %s = name of package, patch, selection ...
		    what = str::form (_("%s is needed by other resolvables"), who.c_str());
		else
		    // TranslatorExplanation %s = name of package, patch, selection ...		    
		    what = str::form (_("%s is needed by %s"), who.c_str(), needed_by->itemsToString(true).c_str());
		details = str::form (_("%s is needed by:\n%s"), who.c_str(), needed_by->itemsToString(false).c_str());
	    }
	    break;
	    case RESOLVER_INFO_TYPE_CONFLICTS_WITH: {
		ResolverInfoConflictsWith_constPtr conflicts_with = dynamic_pointer_cast<const ResolverInfoConflictsWith>(info);
		if (conflicts_with->items().size() >= 1)
		    // TranslatorExplanation %s = name of package, patch, selection ...
		    what = str::form (_("%s conflicts with other resolvables"), who.c_str() );
		else
		    // TranslatorExplanation %s = name of package, patch, selection ...		
		    what = str::form (_("%s conflicts with %s"), who.c_str(), conflicts_with->itemsToString(true).c_str());
		details = str::form (_("%s conflicts with:\n%s"), who.c_str(), conflicts_with->itemsToString(false).c_str());
		ResolverProblem_Ptr problem = new ResolverProblem (what, details);		
		// Uninstall p
		problem->addSolution (new ProblemSolutionUninstall (problem, item));
		if (conflicts_with->items().size() == 1) {
		    // Uninstall q
		    problem->addSolution (new ProblemSolutionUninstall (problem, *(conflicts_with->items().begin())));
		} else {
		    // Uninstall all other
		    PoolItemList conflict_items = conflicts_with->items();
		    problem->addSolution (new ProblemSolutionUninstall (problem, conflict_items));
		}
		// Remove conflict in the resolvable which has to be installed
		problem->addSolution (new ProblemSolutionIgnoreConflicts (problem, item, conflicts_with->capability(),
									  conflicts_with->items())); 
		problems.push_back (problem);
		problem_created = true;
	    }
	    break;
	    case RESOLVER_INFO_TYPE_OBSOLETES: { // no solution; it is only a info
		ResolverInfoObsoletes_constPtr obsoletes = dynamic_pointer_cast<const ResolverInfoObsoletes>(info);
		if (obsoletes->items().size() >= 1)
		    // TranslatorExplanation %s = name of package, patch, selection ...
		    what = str::form (_("%s obsoletes other resolvables"), who.c_str());
		else
		    // TranslatorExplanation %s = name of package, patch, selection ...		
		    what = str::form (_("%s obsoletes %s"), who.c_str(), obsoletes->itemsToString(true).c_str());
		// TranslatorExplanation %s = name of package, patch, selection ...				
		details = str::form (_("%s obsoletes:%s"), who.c_str(), obsoletes->itemsToString(false).c_str());
		details += _("\nThese resolvables will be deleted from the system.");
	    }
	    break;
	    case RESOLVER_INFO_TYPE_DEPENDS_ON: { // no solution; it is only a info
		ResolverInfoDependsOn_constPtr depends_on = dynamic_pointer_cast<const ResolverInfoDependsOn>(info);
		if (depends_on->items().size() >= 1)
		    // TranslatorExplanation %s = name of package, patch, selection ...
		    what = str::form (_("%s depends on other resolvables"), who.c_str(),
				      depends_on->itemsToString(true).c_str());
		else
		    // TranslatorExplanation %s = name of package, patch, selection ...				
		    what = str::form (_("%s depends on %s"), who.c_str(),
				      depends_on->itemsToString(true).c_str());
		// TranslatorExplanation %s = name of package, patch, selection ...				
		details = str::form (_("%s depends on:%s"), who.c_str(), depends_on->itemsToString(false).c_str());		
	    }
	    break;
	    case RESOLVER_INFO_TYPE_CHILD_OF: {				// unused
		ResolverInfoChildOf_constPtr child_of = dynamic_pointer_cast<const ResolverInfoChildOf>(info);
		// TranslatorExplanation: currently it is unused.
		what = _("Child of");
	    }
	    break;
	    case RESOLVER_INFO_TYPE_MISSING_REQ: { // no solution; it is only a info
		ResolverInfoMissingReq_constPtr missing_req = dynamic_pointer_cast<const ResolverInfoMissingReq>(info);
		// TranslatorExplanation %s = dependency
		what = str::form (_("Cannot install %s"), who.c_str());
		// TranslatorExplanation %s = capability		
		details = str::form (_("None provides %s"), missing_req->capability().asString().c_str());
		details += _("\nThere is no resource available which support this requirement.");
	    }
	    break;

 	// from ResolverContext
	    case RESOLVER_INFO_TYPE_INVALID_SOLUTION: {			// Marking this resolution attempt as invalid.
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		what = misc_info->message();
		details = _("Due problems which are described above/below this resolution will not solve all dependencies");
		// no solution available
	    }
	    break;
	    case RESOLVER_INFO_TYPE_UNINSTALLABLE: {			// Marking p as uninstallable
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// Trying to find a concerning conflict

		for (ConflictMap::const_iterator it = collector.conflictMap.begin();
		     it != collector.conflictMap.end(); ++it) {
		    if (it->first == item) {
			what = str::form (_("Cannot install %s because it is conflicting with %s"),
					  who.c_str(),
					  it->second->name().c_str());				
			details = ""; // no further details
			ResolverProblem_Ptr problem = new ResolverProblem (what, details);		
			// Uninstall p
			problem->addSolution (new ProblemSolutionUninstall (problem, item));
			// Uninstall q
			problem->addSolution (new ProblemSolutionUninstall (problem, it->second));
			problems.push_back (problem);
			problem_created = true;
		    }
		}
		if (!problem_created) {
		    // default 
		    what = misc_info->message();
		    // TranslatorExplanation %s = name of package,patch,...		
		    details = str::form (_("%s is not installed and has been marked as uninstallable"), who.c_str());
		    ResolverProblem_Ptr problem = new ResolverProblem (what, details);
		    problem->addSolution (new ProblemSolutionInstall (problem, item)); // Install resolvable again
		    problems.push_back (problem);
		    problem_created = true;
		}
	    }
	    break;
	    case RESOLVER_INFO_TYPE_REJECT_INSTALL: {			// p is scheduled to be installed, but this is not possible because of dependency problems.
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package,patch,...				
		what = str::form (_("Cannot install %s due to dependency problems"), who.c_str());
		details = misc_info->message();
		ResolverProblem_Ptr problem = new ResolverProblem (what, details);
		// Uninstall it; 
		problem->addSolution (new ProblemSolutionUninstall (problem, item));
		// currently no solution concerning "ignore" is available		
		problems.push_back (problem);
		problem_created = true;
	    }
	    break;
	    case RESOLVER_INFO_TYPE_INSTALL_TO_BE_UNINSTALLED: {	// Can't install p since it is already marked as needing to be uninstalled
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package,patch,...				
		what = misc_info->message();
		ResolverProblem_Ptr problem = new ResolverProblem (what, details);
		problem->addSolution (new ProblemSolutionInstall (problem, item)); // Install resolvable again
		problems.push_back (problem);
		problem_created = true;
	    }
	    break;
	    case RESOLVER_INFO_TYPE_INSTALL_UNNEEDED: {			// Can't install p since it is does not apply to this system.
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package,patch,...				
		what = misc_info->message();
		// no solution; it is only a info
	    }
	    break;
	    case RESOLVER_INFO_TYPE_INSTALL_PARALLEL: {			// Can't install p, since a resolvable of the same name is already marked as needing to be installed.
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package,patch,...				
		what = str::form (_("Cannot install %s"), who.c_str());
		details = misc_info->message();
		ResolverProblem_Ptr problem = new ResolverProblem (what, details);
		// Uninstall the item
		ResStatus status = item.status();
		string description = "";
		if (status.isInstalled())
		    // TranslatorExplanation %s = name of package, patch, selection ...
		    description = str::form (_("delete %s"), ResolverInfo::toString (item).c_str());
		else
		    // TranslatorExplanation %s = name of package, patch, selection ...	
		    description = str::form (_("do not install %s"), ResolverInfo::toString (item).c_str());
		problem->addSolution (new ProblemSolutionUninstall (problem, item, description, ""));
		
		// Uninstall the other
		status = misc_info->other().status();
		if (status.isInstalled())
		    // TranslatorExplanation %s = name of package, patch, selection ...
		    description = str::form (_("delete %s"), ResolverInfo::toString (misc_info->other()).c_str());
		else
		    // TranslatorExplanation %s = name of package, patch, selection ...	
		    description = str::form (_("do not install %s"), ResolverInfo::toString (misc_info->other()).c_str());		
		problem->addSolution (new ProblemSolutionUninstall (problem, misc_info->other(), description, ""));
		
		// Ignore it
		problem->addSolution (new ProblemSolutionIgnoreInstalled (problem, item, misc_info->other()));
		problems.push_back (problem);
		problem_created = true;		
	    }
	    break;
	    case RESOLVER_INFO_TYPE_INCOMPLETES: {			// This would invalidate p
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		what = misc_info->message();
		// TranslatorExplanation %s = name of package, patch, selection ...				
		details = str::form (_("%s has unfulfiled requirements"), who.c_str());
		ResolverProblem_Ptr problem = new ResolverProblem (what, details);
		// Uninstall 
		problem->addSolution (new ProblemSolutionUninstall (problem, item));
		problems.push_back (problem);
		problem_created = true;		
	    }
	    break;
	// from QueueItemEstablish
	    case RESOLVER_INFO_TYPE_ESTABLISHING: {			// Establishing p
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		what = misc_info->message();
		// no solution is needed cause it is only a progress indicator
	    }
	    break;
	// from QueueItemInstall
	    case RESOLVER_INFO_TYPE_INSTALLING: {			// Installing p
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		what = misc_info->message();
		// no solution is needed cause it is only a progress indicator		
	    }
	    break;
	    case RESOLVER_INFO_TYPE_UPDATING: {				// Updating p
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		what = misc_info->message();
		// no solution is needed cause it is only a progress indicator		
	    }
	    break;
	    case RESOLVER_INFO_TYPE_SKIPPING: {				// Skipping p, already installed
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		what = misc_info->message();
		// It is only an info and happens while upgrading
	    }
	    break;
	// from QueueItemRequire
	    case RESOLVER_INFO_TYPE_NO_OTHER_PROVIDER: {		// There are no alternative installed providers of c [for p]
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package, patch, selection ...				
		what = str::form (_("%s has missing dependencies"), who.c_str());
		details = misc_info->message();
		ResolverProblem_Ptr problem = new ResolverProblem (what, details);

		// Searching for another item which provides this requires BUT has been set to uninstall
		for (ItemCapabilityMap::const_iterator it = collector.provideAndDeleteMap.begin();
		     it != collector.provideAndDeleteMap.end(); ++it) {
		    if (it->second.matches (misc_info->capability()) == CapMatch::yes) {
			// Do not delete
			problem->addSolution (new ProblemSolutionKeep (problem, it->first));
		    }
		}

		// uninstall
		problem->addSolution (new ProblemSolutionUninstall (problem, item));

		// Unflag require ONLY for this item
		problem->addSolution (new ProblemSolutionIgnoreRequires (problem, item, misc_info->capability()));		

		// Unflag ALL require
		// Evaluating all require Items
		AllRequires info;
		Dep dep( Dep::REQUIRES );

		invokeOnEach( _pool.byCapabilityIndexBegin( misc_info->capability().index(), dep ), // begin()
			      _pool.byCapabilityIndexEnd( misc_info->capability().index(), dep ),   // end()
			      resfilter::ByCapMatch( misc_info->capability() ),
			      functor::functorRef<bool,CapAndItem>(info) );
		if (info.requirers.size() > 1)
		    problem->addSolution (new ProblemSolutionIgnoreRequires (problem, info.requirers, misc_info->capability()));

		problems.push_back (problem);
		problem_created = true;
	    }
	    break;
	    case RESOLVER_INFO_TYPE_NO_PROVIDER: {			// There are no installable providers of c [for p]
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package, patch, selection ...				
		what = str::form (_("%s cannot be installed due to missing dependencies"), who.c_str());		
		details = misc_info->message();
		ResolverProblem_Ptr problem = new ResolverProblem (what, details);
		
		// Searching for another item which provides this requires BUT has been locked
		for (ItemCapabilityMap::const_iterator it = collector.provideAndLockMap.begin();
		     it != collector.provideAndLockMap.end(); ++it) {
		    if (it->second.matches (misc_info->capability()) == CapMatch::yes) {
			// unlock this item
			problem->addSolution (new ProblemSolutionUnlock (problem, it->first));
		    }
		}
		
		// uninstall
		problem->addSolution (new ProblemSolutionUninstall (problem, item)); 
		// ignore requirement
		problem->addSolution (new ProblemSolutionIgnoreRequires (problem, item, misc_info->capability())); 
		problems.push_back (problem);
		problem_created = true;		
	    }
	    break;
	    case RESOLVER_INFO_TYPE_NO_UPGRADE: {			// Upgrade to q to avoid removing p is not possible.
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		what = misc_info->message();
		// It is only an info --> no solution is needed		
	    }
	    break;
	    case RESOLVER_INFO_TYPE_UNINSTALL_PROVIDER: {		// p provides c but is scheduled to be uninstalled
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package, patch, selection ...				
		what =str::form (_("%s fulfil dependencies of %s but will be uninstalled"),
				 misc_info->other()->name().c_str(),
				 who.c_str());
		details = misc_info->message();
		// It is only an info --> no solution is needed
	    }
	    break;
	    case RESOLVER_INFO_TYPE_PARALLEL_PROVIDER: {		// p provides c but another version is already installed
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package, patch, selection ...				
		what = str::form (_("No need to install %s"), misc_info->other()->name().c_str());
		details = misc_info->message();
		// It is only an info --> no solution is needed		
	    }
	    break;
	    case RESOLVER_INFO_TYPE_NOT_INSTALLABLE_PROVIDER: {		// p provides c but is uninstallable
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package, patch, selection ...				
		what = str::form (_("Cannot install %s to fulfil the dependencies of %s"),
				  misc_info->other()->name().c_str(),
				  who.c_str());
		what = misc_info->message();
		// It is only an info --> no solution is needed		
	    }
	    break;
	    case RESOLVER_INFO_TYPE_LOCKED_PROVIDER: {			// p provides c but is locked
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package, patch, selection ...				
		what = str::form (_("Cannot be install %s to fulfil the dependencies of %s"),
				  misc_info->other()->name().c_str(),
				  who.c_str());				
		what = misc_info->message();
		// It is only an info --> no solution is needed		
	    }
	    break;
	    case RESOLVER_INFO_TYPE_CANT_SATISFY: {			// Can't satisfy requirement c
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		what = misc_info->message();
		ResolverProblem_Ptr problem = new ResolverProblem (what, details);
		// uninstall
		problem->addSolution (new ProblemSolutionUninstall (problem, item)); 
		
		// Unflag requirement for this item
		problem->addSolution (new ProblemSolutionIgnoreRequires (problem, item, misc_info->capability()));
		
		// Unflag ALL require
		// Evaluating all require Items
		AllRequires info;
		Dep dep( Dep::REQUIRES );

		invokeOnEach( _pool.byCapabilityIndexBegin( misc_info->capability().index(), dep ), // begin()
			      _pool.byCapabilityIndexEnd( misc_info->capability().index(), dep ),   // end()
			      resfilter::ByCapMatch( misc_info->capability() ),
			      functor::functorRef<bool,CapAndItem>(info) );
		if (info.requirers.size() > 1)
		    problem->addSolution (new ProblemSolutionIgnoreRequires (problem, info.requirers, misc_info->capability()));
		
		problems.push_back (problem);
		problem_created = true;		
	    }
	    break;
	// from QueueItemUninstall
	    case RESOLVER_INFO_TYPE_UNINSTALL_TO_BE_INSTALLED: {	// p is to-be-installed, so it won't be unlinked.
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package, patch, selection ...				
		what = str::form (_("%s will not be uninstalled cause it is still required"), who.c_str());
		details = misc_info->message();
		// It is only an info --> no solution is needed				
	    }
	    break;
	    case RESOLVER_INFO_TYPE_UNINSTALL_INSTALLED: {		// p is required by installed, so it won't be unlinked.
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package, patch, selection ...				
		what = str::form (_("%s will not be uninstalled cause it is still required"), who.c_str());		
		details = misc_info->message();
		// It is only an info --> no solution is needed				
	    }
	    break;
	    case RESOLVER_INFO_TYPE_UNINSTALL_LOCKED: {			// cant uninstall, its locked
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		what = misc_info->message();
		
		if (misc_info->trigger() == ResolverInfoMisc::OBSOLETE) {
		    // TranslatorExplanation %s = name of package, patch, selection ...						    
		    details = str::form (_("%s obsoletes %s. But %s cannot be deleted because it is locked."),
					 misc_info->other()->name().c_str(),
					 who.c_str(), who.c_str());
		}
		
		ResolverProblem_Ptr problem = new ResolverProblem (what, details);
		problem->addSolution (new ProblemSolutionUnlock (problem, item)); // Unlocking resItem
		if (misc_info->trigger() == ResolverInfoMisc::OBSOLETE) {
		    // Ignore obsoletes
		    problem->addSolution (new ProblemSolutionIgnoreObsoletes (problem, item, misc_info->capability(),
									      misc_info->other())); 
		} else {
		    // This is an "default" soltution
		    // keep installed
		    problem->addSolution (new ProblemSolutionKeep (problem, item));
		}
		problems.push_back (problem);
		problem_created = true;
	    }
	    break;
	// from QueueItemConflict
	    case RESOLVER_INFO_TYPE_CONFLICT_CANT_INSTALL: {		// to-be-installed p conflicts with q due to c
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package, patch, selection ...				
		what = str::form (_("Cannot install %s because it is conflicting with %s"),
				  who.c_str(),
				  misc_info->other()->name().c_str());				
		details = misc_info->message();
		ResolverProblem_Ptr problem = new ResolverProblem (what, details);		
		// Uninstall p
		problem->addSolution (new ProblemSolutionUninstall (problem, item));
		// Uninstall q
		problem->addSolution (new ProblemSolutionUninstall (problem, misc_info->other()));
		// Remove conflict in the resolvable which has to be installed
		problem->addSolution (new ProblemSolutionIgnoreConflicts (problem, item, misc_info->other_capability(),
									  misc_info->other())); 
		problems.push_back (problem);
		problem_created = true;
		
	    }
	    break;
	    case RESOLVER_INFO_TYPE_CONFLICT_UNINSTALLABLE: {		// uninstalled p is marked uninstallable it conflicts [with q] due to c
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package, patch, selection ...				
		what = str::form (_("%s is uninstallable due to conflicts with %s"),
				who.c_str(),
				misc_info->other()->name().c_str());				
		details = misc_info->message();
		// It is only an info --> no solution is needed
	    }
	    break;
	}
	if (!problem_created) {
	    ResolverProblem_Ptr problem = new ResolverProblem (what, details);
	    problems.push_back (problem);
	}
    }
    if (problems.empty()) {
	context->spewInfo();
    }
    return problems;
}

void
Resolver::applySolutions (const ProblemSolutionList & solutions)
{
    for (ProblemSolutionList::const_iterator iter = solutions.begin();
	 iter != solutions.end(); ++iter) {
	ProblemSolution_Ptr solution = *iter;
	if (!solution->apply (*this))
	    break;
    }    
}

///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

