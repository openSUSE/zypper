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
#include "zypp/solver/detail/Testcase.h"
#include "zypp/solver/detail/SATResolver.h"

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
#include "zypp/sat/Transaction.h"
#include "zypp/ResolverProblem.h"

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


std::ostream & Resolver::dumpOn( std::ostream & os ) const
{
  os << "<resolver>" << endl;
  #define OUTS(t) os << "  " << #t << ":\t" << t << endl;
  OUTS( _forceResolve );
  OUTS( _upgradeMode );
  OUTS( _updateMode );
  OUTS( _verifying );
  OUTS( _onlyRequires );
  OUTS( _allowVendorChange );
  OUTS( _solveSrcPackages );
  OUTS( _cleandepsOnRemove );
  OUTS( _ignoreAlreadyRecommended );
  #undef OUT
  return os << "<resolver/>";
}


//---------------------------------------------------------------------------

Resolver::Resolver (const ResPool & pool)
    : _pool(pool)
    , _satResolver(NULL)
    , _poolchanged(_pool.serial() )
    , _forceResolve		(false)
    , _upgradeMode		(false)
    , _updateMode		(false)
    , _verifying		(false)
    , _onlyRequires		( ZConfig::instance().solver_onlyRequires() )
    , _allowVendorChange	( ZConfig::instance().solver_allowVendorChange() )
    , _solveSrcPackages		( false )
    , _cleandepsOnRemove	( ZConfig::instance().solver_cleandepsOnRemove() )
    , _ignoreAlreadyRecommended	( true )

{
    sat::Pool satPool( sat::Pool::instance() );
    _satResolver = new SATResolver(_pool, satPool.get());
}


Resolver::~Resolver()
{
  delete _satResolver;
}

//---------------------------------------------------------------------------
// forward flags too SATResolver
#define ZOLV_FLAG_TRIBOOL( ZSETTER, ZGETTER, ZVARNAME, ZVARDEFAULT )			\
    void Resolver::ZSETTER( TriBool state_r )						\
    { _satResolver->ZVARNAME = indeterminate(state_r) ? ZVARDEFAULT : bool(state_r); }	\
    bool Resolver::ZGETTER() const							\
    { return _satResolver->ZVARNAME; }							\

ZOLV_FLAG_TRIBOOL( dupSetAllowDowngrade,	dupAllowDowngrade,	_dup_allowdowngrade,	true )
ZOLV_FLAG_TRIBOOL( dupSetAllowNameChange,	dupAllowNameChange,	_dup_allownamechange,	true )
ZOLV_FLAG_TRIBOOL( dupSetAllowArchChange,	dupAllowArchChange,	_dup_allowarchchange,	true )
ZOLV_FLAG_TRIBOOL( dupSetAllowVendorChange,	dupAllowVendorChange,	_dup_allowvendorchange,	true )

#undef ZOLV_FLAG_TRIBOOL
//---------------------------------------------------------------------------

void Resolver::setAllowVendorChange( TriBool state_r )
{
  _allowVendorChange = indeterminate(state_r) ? ZConfig::instance().solver_allowVendorChange() : bool(state_r);
}

void Resolver::setOnlyRequires( TriBool state_r )
{
  _onlyRequires = indeterminate(state_r) ? ZConfig::instance().solver_onlyRequires() : bool(state_r);
}

void Resolver::setCleandepsOnRemove( TriBool state_r )
{
  _cleandepsOnRemove = indeterminate(state_r) ? ZConfig::instance().solver_cleandepsOnRemove() : bool(state_r);
}

//---------------------------------------------------------------------------

ResPool Resolver::pool() const
{ return _pool; }

void Resolver::reset( bool keepExtras )
{
    _verifying = false;

    if (!keepExtras) {
      _extra_requires.clear();
      _extra_conflicts.clear();
    }

    _isInstalledBy.clear();
    _installs.clear();
    _satifiedByInstalled.clear();
    _installedSatisfied.clear();
}

bool Resolver::doUpgrade()
{
  // Setting Resolver to upgrade mode. SAT solver will do the update
  _upgradeMode = true;
  return resolvePool();
}

void Resolver::doUpdate()
{
    _updateMode = true;
    return _satResolver->doUpdate();
}

PoolItemList Resolver::problematicUpdateItems() const
{ return _satResolver->problematicUpdateItems(); }

void Resolver::addExtraRequire( const Capability & capability )
{ _extra_requires.insert (capability); }

void Resolver::removeExtraRequire( const Capability & capability )
{ _extra_requires.erase (capability); }

void Resolver::addExtraConflict( const Capability & capability )
{ _extra_conflicts.insert (capability); }

void Resolver::removeExtraConflict( const Capability & capability )
{ _extra_conflicts.erase (capability); }

void Resolver::removeQueueItem( SolverQueueItem_Ptr item )
{
    bool found = false;
    for (SolverQueueItemList::const_iterator iter = _added_queue_items.begin();
	 iter != _added_queue_items.end(); iter++) {
	if (*iter == item) {
	    _added_queue_items.remove(*iter);
	    found = true;
	    break;
	}
    }
    if (!found) {
	_removed_queue_items.push_back (item);
	_removed_queue_items.unique ();
    }
}

void Resolver::addQueueItem( SolverQueueItem_Ptr item )
{
    bool found = false;
    for (SolverQueueItemList::const_iterator iter = _removed_queue_items.begin();
	 iter != _removed_queue_items.end(); iter++) {
	if (*iter == item) {
	    _removed_queue_items.remove(*iter);
	    found = true;
	    break;
	}
    }
    if (!found) {
	_added_queue_items.push_back (item);
	_added_queue_items.unique ();
    }
}

void Resolver::addWeak( const PoolItem & item )
{ _addWeak.push_back( item ); }

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


bool Resolver::verifySystem()
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
void Resolver::undo()
{
    UndoTransact info(ResStatus::APPL_LOW);
    MIL << "*** undo ***" << endl;
    invokeOnEach ( _pool.begin(), _pool.end(),
		   resfilter::ByTransact( ),			// collect transacts from Pool to resolver queue
		   functor::functorRef<bool,PoolItem>(info) );
    //  Regard dependencies of the item weak onl
    _addWeak.clear();

    // Additional QueueItems which has to be regarded by the solver
    _removed_queue_items.clear();
    _added_queue_items.clear();

    return;
}

void Resolver::solverInit()
{
    // Solving with libsolv
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

    _satResolver->setFixsystem			( isVerifyingMode() );
    _satResolver->setIgnorealreadyrecommended	( ignoreAlreadyRecommended() );
    _satResolver->setOnlyRequires		( onlyRequires() );
    _satResolver->setAllowdowngrade		(false);
    _satResolver->setAllowarchchange		(false);
    _satResolver->setAllowvendorchange		( allowVendorChange() );
    _satResolver->setAllowuninstall		( forceResolve() );
    _satResolver->setUpdatesystem		(false);
    _satResolver->setNoupdateprovide		(false);
    _satResolver->setDosplitprovides		(true);
    _satResolver->setSolveSrcPackages		( solveSrcPackages() );
    _satResolver->setCleandepsOnRemove		( cleandepsOnRemove() );

    _satResolver->setDistupgrade		(_upgradeMode);
    if (_upgradeMode) {
      // may overwrite some settings
      _satResolver->setDistupgrade_removeunsupported	(false);
    }

    // Resetting additional solver information
    _isInstalledBy.clear();
    _installs.clear();
    _satifiedByInstalled.clear();
    _installedSatisfied.clear();
}

bool Resolver::resolvePool()
{
    solverInit();
    return _satResolver->resolvePool(_extra_requires, _extra_conflicts, _addWeak, _upgradeRepos );
}

bool Resolver::resolveQueue( solver::detail::SolverQueueItemList & queue )
{
    solverInit();

    // add/remove additional SolverQueueItems
    for (SolverQueueItemList::const_iterator iter = _removed_queue_items.begin();
	 iter != _removed_queue_items.end(); iter++) {
	for (SolverQueueItemList::const_iterator iterQueue = queue.begin(); iterQueue != queue.end(); iterQueue++) {
	    if ( (*iterQueue)->cmp(*iter) == 0) {
		MIL << "remove from queue" << *iter;
		queue.remove(*iterQueue);
		break;
	    }
	}
    }

    for (SolverQueueItemList::const_iterator iter = _added_queue_items.begin();
	 iter != _added_queue_items.end(); iter++) {
	bool found = false;
	for (SolverQueueItemList::const_iterator iterQueue = queue.begin(); iterQueue != queue.end(); iterQueue++) {
	    if ( (*iterQueue)->cmp(*iter) == 0) {
		found = true;
		break;
	    }
	}
	if (!found) {
	    MIL << "add to queue" << *iter;
	    queue.push_back(*iter);
	}
    }

    // The application has to take care to write these solutions back to e.g. selectables in order
    // give the user a chance for changing these decisions again.
    _removed_queue_items.clear();
    _added_queue_items.clear();

    return _satResolver->resolveQueue(queue, _addWeak);
}

sat::Transaction Resolver::getTransaction()
{
  // FIXME: That's an ugly way of pushing autoInstalled into the transaction.
  sat::Transaction ret( sat::Transaction::loadFromPool );
  ret.autoInstalled( _satResolver->autoInstalled() );
  return ret;
}


//----------------------------------------------------------------------------
// Getting more information about the solve results

ResolverProblemList Resolver::problems() const
{
  MIL << "Resolver::problems()" << endl;
  return _satResolver->problems();
}

void Resolver::applySolutions( const ProblemSolutionList & solutions )
{
  for_( iter, solutions.begin(), solutions.end() )
  {
    ProblemSolution_Ptr solution = *iter;
    if ( !solution->apply( *this ) )
      break;
  }
}

void Resolver::collectResolverInfo()
{
    if ( _satResolver
	 && _isInstalledBy.empty()
	 && _installs.empty()) {

	// generating new
	PoolItemList itemsToInstall = _satResolver->resultItemsToInstall();

	for (PoolItemList::const_iterator instIter = itemsToInstall.begin();
	     instIter != itemsToInstall.end(); instIter++) {
	    // Requires
	    for (Capabilities::const_iterator capIt = (*instIter)->dep (Dep::REQUIRES).begin(); capIt != (*instIter)->dep (Dep::REQUIRES).end(); ++capIt)
	    {
		sat::WhatProvides possibleProviders(*capIt);
		for_( iter, possibleProviders.begin(), possibleProviders.end() ) {
		    PoolItem provider = ResPool::instance().find( *iter );

		    // searching if this provider will already be installed
		    bool found = false;
		    bool alreadySetForInstallation = false;
		    ItemCapKindMap::const_iterator pos = _isInstalledBy.find(provider);
		    while (pos != _isInstalledBy.end()
			   && pos->first == provider
			   && !found) {
			alreadySetForInstallation = true;
			ItemCapKind capKind = pos->second;
			if (capKind.item == *instIter)  found = true;
			pos++;
		    }

		    if (!found
			&& provider.status().isToBeInstalled()) {
			if (provider.status().isBySolver()) {
			    ItemCapKind capKindisInstalledBy( *instIter, *capIt, Dep::REQUIRES, !alreadySetForInstallation );
			    _isInstalledBy.insert (make_pair( provider, capKindisInstalledBy));
			} else {
			    // no initial installation cause it has been set be e.g. user
			    ItemCapKind capKindisInstalledBy( *instIter, *capIt, Dep::REQUIRES, false );
			    _isInstalledBy.insert (make_pair( provider, capKindisInstalledBy));
			}
			ItemCapKind capKindisInstalledBy( provider, *capIt, Dep::REQUIRES, !alreadySetForInstallation );
			_installs.insert (make_pair( *instIter, capKindisInstalledBy));
		    }

		    if (provider.status().staysInstalled()) { // Is already satisfied by an item which is installed
			ItemCapKind capKindisInstalledBy( provider, *capIt, Dep::REQUIRES, false );
			_satifiedByInstalled.insert (make_pair( *instIter, capKindisInstalledBy));

			ItemCapKind installedSatisfied( *instIter, *capIt, Dep::REQUIRES, false );
			_installedSatisfied.insert (make_pair( provider, installedSatisfied));
		    }
		}
	    }

	    if (!(_satResolver->onlyRequires())) {
		//Recommends
		for (Capabilities::const_iterator capIt = (*instIter)->dep (Dep::RECOMMENDS).begin(); capIt != (*instIter)->dep (Dep::RECOMMENDS).end(); ++capIt)
		{
		    sat::WhatProvides possibleProviders(*capIt);
		    for_( iter, possibleProviders.begin(), possibleProviders.end() ) {
			PoolItem provider = ResPool::instance().find( *iter );

			// searching if this provider will already be installed
			bool found = false;
			bool alreadySetForInstallation = false;
			ItemCapKindMap::const_iterator pos = _isInstalledBy.find(provider);
			while (pos != _isInstalledBy.end()
			       && pos->first == provider
			       && !found) {
			    alreadySetForInstallation = true;
			    ItemCapKind capKind = pos->second;
			    if (capKind.item == *instIter)  found = true;
			    pos++;
			}

			if (!found
			    && provider.status().isToBeInstalled()) {
			    if (provider.status().isBySolver()) {
				ItemCapKind capKindisInstalledBy( *instIter, *capIt, Dep::RECOMMENDS, !alreadySetForInstallation );
				_isInstalledBy.insert (make_pair( provider, capKindisInstalledBy));
			    } else {
				// no initial installation cause it has been set be e.g. user
				ItemCapKind capKindisInstalledBy( *instIter, *capIt, Dep::RECOMMENDS, false );
				_isInstalledBy.insert (make_pair( provider, capKindisInstalledBy));
			    }
			    ItemCapKind capKindisInstalledBy( provider, *capIt, Dep::RECOMMENDS, !alreadySetForInstallation );
			    _installs.insert (make_pair( *instIter, capKindisInstalledBy));
			}

			if (provider.status().staysInstalled()) { // Is already satisfied by an item which is installed
			    ItemCapKind capKindisInstalledBy( provider, *capIt, Dep::RECOMMENDS, false );
			    _satifiedByInstalled.insert (make_pair( *instIter, capKindisInstalledBy));

			    ItemCapKind installedSatisfied( *instIter, *capIt, Dep::RECOMMENDS, false );
			    _installedSatisfied.insert (make_pair( provider, installedSatisfied));
			}
		    }
		}

		//Supplements
		for (Capabilities::const_iterator capIt = (*instIter)->dep (Dep::SUPPLEMENTS).begin(); capIt != (*instIter)->dep (Dep::SUPPLEMENTS).end(); ++capIt)
		{
		    sat::WhatProvides possibleProviders(*capIt);
		    for_( iter, possibleProviders.begin(), possibleProviders.end() ) {
			PoolItem provider = ResPool::instance().find( *iter );
			// searching if this item will already be installed
			bool found = false;
			bool alreadySetForInstallation = false;
			ItemCapKindMap::const_iterator pos = _isInstalledBy.find(*instIter);
			while (pos != _isInstalledBy.end()
			       && pos->first == *instIter
			       && !found) {
			    alreadySetForInstallation = true;
			    ItemCapKind capKind = pos->second;
			    if (capKind.item == provider)  found = true;
			    pos++;
			}

			if (!found
			    && instIter->status().isToBeInstalled()) {
			    if (instIter->status().isBySolver()) {
				ItemCapKind capKindisInstalledBy( provider, *capIt, Dep::SUPPLEMENTS, !alreadySetForInstallation );
				_isInstalledBy.insert (make_pair( *instIter, capKindisInstalledBy));
			    } else {
				// no initial installation cause it has been set be e.g. user
				ItemCapKind capKindisInstalledBy( provider, *capIt, Dep::SUPPLEMENTS, false );
				_isInstalledBy.insert (make_pair( *instIter, capKindisInstalledBy));
			    }
			    ItemCapKind capKindisInstalledBy( *instIter, *capIt, Dep::SUPPLEMENTS, !alreadySetForInstallation );
			    _installs.insert (make_pair( provider, capKindisInstalledBy));
			}

			if (instIter->status().staysInstalled()) { // Is already satisfied by an item which is installed
			    ItemCapKind capKindisInstalledBy( *instIter, *capIt, Dep::SUPPLEMENTS, !alreadySetForInstallation );
			    _satifiedByInstalled.insert (make_pair( provider, capKindisInstalledBy));

			    ItemCapKind installedSatisfied( provider, *capIt, Dep::SUPPLEMENTS, false );
			    _installedSatisfied.insert (make_pair( *instIter, installedSatisfied));
			}
		    }
		}
   	    }
	}
    }
}


ItemCapKindList Resolver::isInstalledBy( const PoolItem & item )
{
    ItemCapKindList ret;
    collectResolverInfo();

    for (ItemCapKindMap::const_iterator iter = _isInstalledBy.find(item); iter != _isInstalledBy.end();) {
	ItemCapKind info = iter->second;
	PoolItem iterItem = iter->first;
	if (iterItem == item) {
	    ret.push_back(info);
	    iter++;
	} else {
	    // exit
	    iter = _isInstalledBy.end();
	}
    }
    return ret;
}

ItemCapKindList Resolver::installs( const PoolItem & item )
{
    ItemCapKindList ret;
    collectResolverInfo();

    for (ItemCapKindMap::const_iterator iter = _installs.find(item); iter != _installs.end();) {
	ItemCapKind info = iter->second;
	PoolItem iterItem = iter->first;
	if (iterItem == item) {
	    ret.push_back(info);
	    iter++;
	} else {
	    // exit
	    iter = _installs.end();
	}
    }
    return ret;
}

ItemCapKindList Resolver::satifiedByInstalled( const PoolItem & item )
{
    ItemCapKindList ret;
    collectResolverInfo();

    for (ItemCapKindMap::const_iterator iter = _satifiedByInstalled.find(item); iter != _satifiedByInstalled.end();) {
	ItemCapKind info = iter->second;
	PoolItem iterItem = iter->first;
	if (iterItem == item) {
	    ret.push_back(info);
	    iter++;
	} else {
	    // exit
	    iter = _satifiedByInstalled.end();
	}
    }
    return ret;
}

ItemCapKindList Resolver::installedSatisfied( const PoolItem & item )
{
    ItemCapKindList ret;
    collectResolverInfo();

    for (ItemCapKindMap::const_iterator iter = _installedSatisfied.find(item); iter != _installedSatisfied.end();) {
	ItemCapKind info = iter->second;
	PoolItem iterItem = iter->first;
	if (iterItem == item) {
	    ret.push_back(info);
	    iter++;
	} else {
	    // exit
	    iter = _installedSatisfied.end();
	}
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

