/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Resolver.cc
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
#include <boost/static_assert.hpp>

#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/Helper.h"
#include "zypp/solver/detail/Testcase.h"

#include "zypp/Capabilities.h"
#include "zypp/ZConfig.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/sat/Pool.h"
#include "zypp/sat/Solvable.h"
#include "zypp/sat/SATResolver.h"

#define MAXSOLVERRUNS 5

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

IMPL_PTR_TYPE(Resolver);


//---------------------------------------------------------------------------


std::ostream &
Resolver::dumpOn( std::ostream & os ) const
{
    return os << "<resolver/>";
}


//---------------------------------------------------------------------------

Resolver::Resolver (const ResPool & pool)
    : _pool(pool)
    , _satResolver(NULL)
    , _poolchanged(_pool.serial() )
    , _forceResolve(false)
    , _upgradeMode(false)
    , _verifying(false)
    , _onlyRequires(DEFAULT)

{
    sat::Pool satPool( sat::Pool::instance() );
    _satResolver = new SATResolver(_pool, satPool.get());
}


Resolver::~Resolver()
{
}

//---------------------------------------------------------------------------

ResPool
Resolver::pool (void) const
{
    return _pool;
}

void
Resolver::reset (bool keepExtras )
{
    _verifying = false;    

    if (!keepExtras) {
      _extra_requires.clear();
      _extra_conflicts.clear();
    }
}

bool
Resolver::doUpdate()
{
    if (_satResolver) {
	return _satResolver->doUpdate();
    } else {
	ERR << "SAT solver has not been initialized." << endl;
	return false;
    }
}

void
Resolver::addExtraRequire (const Capability & capability)
{
    _extra_requires.insert (capability);
}

void
Resolver::removeExtraRequire (const Capability & capability)
{
    _extra_requires.erase (capability);
}

void
Resolver::addExtraConflict (const Capability & capability)
{
    _extra_conflicts.insert (capability);
}

void
Resolver::removeExtraConflict (const Capability & capability)
{
    _extra_conflicts.erase (capability);
}

void
Resolver::addIgnoreConflict (const PoolItem item,
		   const Capability & capability)
{
    _ignoreConflicts.insert(make_pair(item, capability));
}

void
Resolver::addIgnoreRequires (const PoolItem item,
			     const Capability & capability)
{
    _ignoreRequires.insert(make_pair(item, capability));
}

void
Resolver::addIgnoreObsoletes (const PoolItem item,
			      const Capability & capability)
{
    _ignoreObsoletes.insert(make_pair(item, capability));
}

void
Resolver::addIgnoreInstalledItem (const PoolItem item)
{
    _ignoreInstalledItem.push_back (item);
}

void
Resolver::addIgnoreArchitectureItem (const PoolItem item)
{
    _ignoreArchitectureItem.push_back (item);
}

void
Resolver::addIgnoreVendorItem (const PoolItem item)
{
    _ignoreVendorItem.push_back (item);
}

//---------------------------------------------------------------------------

struct UndoTransact : public resfilter::PoolItemFilterFunctor
{
    ResStatus::TransactByValue resStatus;
    UndoTransact ( const ResStatus::TransactByValue &status)
	:resStatus(status)
    { }

    bool operator()( PoolItem item )		// only transacts() items go here
    {
	item.status().resetTransact( resStatus );// clear any solver/establish transactions
	return true;
    }
};


struct DoTransact : public resfilter::PoolItemFilterFunctor
{
    ResStatus::TransactByValue resStatus;
    DoTransact ( const ResStatus::TransactByValue &status)
	:resStatus(status)
    { }

    bool operator()( PoolItem item )		// only transacts() items go here
    {
	item.status().setTransact( true, resStatus );
	return true;
    }
};


bool
Resolver::verifySystem ()
{
    UndoTransact resetting (ResStatus::APPL_HIGH);

    _DEBUG ("Resolver::verifySystem() ");
    
    _verifying = true;    

    invokeOnEach ( _pool.begin(), _pool.end(),
		   resfilter::ByTransact( ),			// Resetting all transcations
		   functor::functorRef<bool,PoolItem>(resetting) );

    return resolvePool();
}


//----------------------------------------------------------------------------
// undo

void
Resolver::undo(void)
{
    UndoTransact info(ResStatus::APPL_LOW);
    MIL << "*** undo ***" << endl;
    invokeOnEach ( _pool.begin(), _pool.end(),
		   resfilter::ByTransact( ),			// collect transacts from Pool to resolver queue
		   functor::functorRef<bool,PoolItem>(info) );
    // These conflict should be ignored of the concering item
    _ignoreConflicts.clear();
    // These requires should be ignored of the concering item
    _ignoreRequires.clear();
    // These obsoletes should be ignored of the concering item
    _ignoreObsoletes.clear();
    // Ignore architecture of the item
    _ignoreArchitecture.clear();
    // Ignore the status "installed" of the item
    _ignoreInstalledItem.clear();
    // Ignore the architecture of the item
    _ignoreArchitectureItem.clear();
    // Ignore the vendor of the item
    _ignoreVendorItem.clear();


    return;
}


bool
Resolver::resolvePool()
{

    // Solving with the satsolver
        static bool poolDumped = false;
	MIL << "-------------- Calling SAT Solver -------------------" << endl;
	if ( getenv("ZYPP_FULLLOG") ) {
	    Testcase testcase("/var/log/YaST2/autoTestcase");
	    if (!poolDumped) {
		testcase.createTestcase (*this, true, false); // dump pool
		poolDumped = true;
	    } else {
		testcase.createTestcase (*this, false, false); // write control file only
	    }
	}
#if 0
	MIL << "------SAT-Pool------" << endl;
	for (sat::Pool::SolvableIterator i = satPool.solvablesBegin();
	     i != satPool.solvablesEnd(); i++ ) {
	    MIL << *i << " ID: " << i->id() << endl;
	}
	MIL << "------SAT-Pool end------" << endl;
#endif
	_satResolver->setFixsystem(false);
	_satResolver->setAllowdowngrade(false);
	_satResolver->setAllowarchchange(false);
	_satResolver->setAllowvendorchange(false);
	_satResolver->setAllowuninstall(false);
	_satResolver->setUpdatesystem(false);
	_satResolver->setAllowvirtualconflicts(false);
	_satResolver->setNoupdateprovide(true);
	_satResolver->setDosplitprovides(false);
	
	if (_upgradeMode) {
	    _satResolver->setAllowdowngrade(true);
	    _satResolver->setAllowarchchange(true);
	    _satResolver->setUpdatesystem(true);
	    _satResolver->setDosplitprovides(true);   
	}

	if (_forceResolve)
	    _satResolver->setAllowuninstall(true);
	
 	switch (_onlyRequires) {
	    case DEFAULT:
		_satResolver->setOnlyRequires(ZConfig::instance().solver_onlyRequires());
	    case TRUE:
		_satResolver->setOnlyRequires(true);
	    case FALSE:
		_satResolver->setOnlyRequires(false);
	}

	if (_verifying)
	    _satResolver->setFixsystem(true);
	
	return _satResolver->resolvePool(_extra_requires, _extra_conflicts);
}


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Resolver::checkUnmaintainedItems
//	METHOD TYPE : 
//
//	DESCRIPTION : Unmaintained packages which does not fit to 
//                    the updated system (broken dependencies) will be
//                    deleted.
//
void Resolver::checkUnmaintainedItems () {
    int solverRuns = 1;
    MIL << "Checking unmaintained items....." << endl;

    while (!resolvePool() && solverRuns++ < MAXSOLVERRUNS) {
	ResolverProblemList problemList = problems();
	ProblemSolutionList solutionList;
	PoolItemList problemItemList;	

	for (ResolverProblemList::iterator iter = problemList.begin(); iter != problemList.end(); ++iter) {
	    ResolverProblem problem = **iter;
	    DBG << "Problem:" << endl;
	    DBG << problem.description() << endl;
	    DBG << problem.details() << endl;

	    ProblemSolutionList solutions = problem.solutions();
	    for (ProblemSolutionList::const_iterator iterSolution = solutions.begin();
		 iterSolution != solutions.end(); ++iterSolution) {
		ProblemSolution_Ptr solution = *iterSolution;
		DBG << "   Solution:" << endl;
		DBG << "      " << solution->description() << endl;
		DBG << "      " << solution->details() << endl;		
		solver::detail::CSolutionActionList actionList = solution->actions();
		bool fitUnmaintained = false;
		PoolItemList deletedItems;
		for (CSolutionActionList::const_iterator iterActions = actionList.begin();
		     iterActions != actionList.end(); ++iterActions) {
		    TransactionSolutionAction_constPtr transactionAction = dynamic_pointer_cast<const TransactionSolutionAction>(*iterActions);
		    if (transactionAction &&
			transactionAction->action() == REMOVE
			&& _unmaintained_items.find(transactionAction->item()) != _unmaintained_items.end()) {
			// The solution contains unmaintained items ONLY which will be deleted. So take this solution
			fitUnmaintained = true;
			deletedItems.push_back (transactionAction->item());
		    } else {
			fitUnmaintained = false;
		    }
		}
		if (fitUnmaintained) {
		    MIL << "Problem:" << endl;
		    MIL << problem.description() << endl;
		    MIL << problem.details() << endl;
		    MIL << "Will be solved by removing unmaintained package(s)............" << endl;
		    MIL << "   Solution:" << endl;
		    MIL << "      " << solution->description() << endl;
		    MIL << "      " << solution->details() << endl;				    
		    solutionList.push_back (solution);
		    problemItemList.insert (problemItemList.end(), deletedItems.begin(), deletedItems.end() );
		    break; // not regarding the other solutions
		}
	    }
	}

	if (!solutionList.empty()) {
	    applySolutions (solutionList);
	    // list of problematic items after doUpgrade() which is show to the user
	    _problem_items.insert (_problem_items.end(), problemItemList.begin(), problemItemList.end());
	    _problem_items.unique();
	} else {
	    // break cause there is no other solution available by the next run
	    solverRuns = MAXSOLVERRUNS;
	}
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

