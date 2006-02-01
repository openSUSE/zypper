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

// set resolvables with errors

typedef struct {
    ProblemMap problems;
} ResItemCollector;


static void
collector_cb (ResolverInfo_Ptr info, void *data)
{
    ResItemCollector *collector = (ResItemCollector *)data;
    PoolItem_Ref item = info->affected();
    if (item) {
	collector->problems[item] = info;
    }
}


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
	WAR << "No solver problems, other error" << endl;
    }

    ResolverContext_Ptr context = invalid.front()->context();

    ResItemCollector collector;
    context->foreachInfo (PoolItem(), RESOLVER_INFO_PRIORITY_VERBOSE, collector_cb, &collector);

    for (ProblemMap::const_iterator iter = collector.problems.begin(); iter != collector.problems.end(); ++iter) {
	PoolItem_Ref item = iter->first;
	ResolverInfo_Ptr info = iter->second;

	bool problem_created = false;

	DBG << info;
	if (!(info->error())) {
	    DBG << "; It is not an error --> ignoring" << endl;
	    continue; // only errors are important
	} else{
	    DBG << "; Evaluate solutions..." << endl;
	}
	
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
	    case RESOLVER_INFO_TYPE_CONFLICTS_WITH: { // no solution; it is only a info
		ResolverInfoConflictsWith_constPtr conflicts_with = dynamic_pointer_cast<const ResolverInfoConflictsWith>(info);
		if (conflicts_with->items().size() >= 1)
		    // TranslatorExplanation %s = name of package, patch, selection ...
		    what = str::form (_("%s conflicts with other resolvables"), who.c_str() );
		else
		    // TranslatorExplanation %s = name of package, patch, selection ...		
		    what = str::form (_("%s conflicts with %s"), who.c_str(), conflicts_with->itemsToString(true).c_str());
		details = str::form (_("%s conflicts with:\n%s"), who.c_str(), conflicts_with->itemsToString(false).c_str());		
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
		what = misc_info->message();
		// TranslatorExplanation %s = name of package,patch,...		
		details = str::form (_("%s is not installed and has been marked as uninstallable"), who.c_str());
		// it is only a info --> no solution needed
	    }
	    break;
	    case RESOLVER_INFO_TYPE_REJECT_INSTALL: {			// p is scheduled to be installed, but this is not possible because of dependency problems.
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package,patch,...				
		what = str::form (_("Cannot install %s due dependency problems"), who.c_str());
		details = misc_info->message();
		// currently no solution available
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
		// currently no solution available		
	    }
	    break;
	    case RESOLVER_INFO_TYPE_INSTALL_PARALLEL: {			// Can't install p, since a resolvable of the same name is already marked as needing to be installed.
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package,patch,...				
		what = str::form (_("Cannot install %s"), who.c_str());
		details = misc_info->message();
		// currently no solution available cause I do not know the other resolvable.
		// If it would be available we could replace these resolvable by a solution.
	    }
	    break;
	    case RESOLVER_INFO_TYPE_INCOMPLETES: {			// This would invalidate p
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		what = misc_info->message();
		// TranslatorExplanation %s = name of package, patch, selection ...				
		details = str::form (_("%s has unfulfiled requirements"), who.c_str());
		// currently no solution available				
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
		// currently no solution available	
	    }
	    break;
	// from QueueItemRequire
	    case RESOLVER_INFO_TYPE_NO_OTHER_PROVIDER: {		// There are no alternative installed providers of c [for p]
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package, patch, selection ...				
		what = str::form (_("%s cannot be uninstalled due missing dependencies"), who.c_str());
		details = misc_info->message();
		ResolverProblem_Ptr problem = new ResolverProblem (what, details);
		// Add dummy provides
		problem->addSolution (new ProblemSolutionIgnore (problem, Dep::REQUIRES, item, misc_info->capability())); 
		problems.push_back (problem);
		problem_created = true;
	    }
	    break;
	    case RESOLVER_INFO_TYPE_NO_PROVIDER: {			// There are no installable providers of c [for p]
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package, patch, selection ...				
		what = str::form (_("%s cannot be installed due missing dependencies"), who.c_str());		
		details = misc_info->message();
		ResolverProblem_Ptr problem = new ResolverProblem (what, details);
		// Add dummy provides
		problem->addSolution (new ProblemSolutionIgnore (problem, Dep::REQUIRES, item, misc_info->capability())); 
		problems.push_back (problem);
		problem_created = true;		
	    }
	    break;
	    case RESOLVER_INFO_TYPE_NO_UPGRADE: {			// Upgrade to q to avoid removing p is not possible.
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		what = misc_info->message();
		// The reason is not known. So no solution available
	    }
	    break;
	    case RESOLVER_INFO_TYPE_UNINSTALL_PROVIDER: {		// p provides c but is scheduled to be uninstalled
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package, patch, selection ...				
		ostringstream other_str;
		other_str << misc_info->other();
		what =str::form (_("%s fulfil dependencies of %s but will be uninstalled"),
				 other_str.str().c_str(),
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
		// Add dummy provides
		problem->addSolution (new ProblemSolutionIgnore (problem, Dep::REQUIRES, item, misc_info->capability())); 
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
		ResolverProblem_Ptr problem = new ResolverProblem (what, details);
		problem->addSolution (new ProblemSolutionUnlock (problem, item)); // Unlocking resItem
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
#if 1
		// It is only an info --> no solution is needed
#else		
		// Uninstall p
		problem->addSolution (new ProblemSolutionUninstall (problem, resItem));
		// Remove conflict in the resolvable which has to be installed
		problem->addSolution (new ProblemSolutionIgnore (problem, Dep::CONFLICTS, resItem, misc_info->capability(),
								 misc_info->other())); 
		problems.push_back (problem);
		problem_created = true;
#endif
		
	    }
	    break;
	    case RESOLVER_INFO_TYPE_CONFLICT_UNINSTALLABLE: {		// uninstalled p is marked uninstallable it conflicts [with q] due to c
		ResolverInfoMisc_constPtr misc_info = dynamic_pointer_cast<const ResolverInfoMisc>(info);
		// TranslatorExplanation %s = name of package, patch, selection ...				
		what = str::form (_("%s is uninstallable due conflicts with %s"),
				who.c_str(),
				misc_info->other()->name().c_str());				
		details = misc_info->message();
#if 1
		// It is only an info --> no solution is needed
#else
		// Uninstall p
		problem->addSolution (new ProblemSolutionUninstall (problem, resItem));
		// Remove conflict in the resolvable which has to be installed
		problem->addSolution (new ProblemSolutionIgnore (problem, Dep::CONFLICTS, resItem, misc_info->capability(),
								 misc_info->other())); 
		problems.push_back (problem);
		problem_created = true;		
#endif
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


bool
Resolver::applySolutions (const ProblemSolutionList & solutions)
{
    bool ret = true;
    for (ProblemSolutionList::const_iterator iter = solutions.begin();
	 iter != solutions.end(); ++iter) {
	ProblemSolution_Ptr solution = *iter;
	ret = solution->apply (*this);
	if (ret == false)
	    break;
    }    
    return ret;
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

