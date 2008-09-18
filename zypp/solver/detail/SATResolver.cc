/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* SATResolver.cc
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
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */
#include <sstream>
#include "zypp/solver/detail/Helper.h"
#include "zypp/base/String.h"
#include "zypp/Capability.h"
#include "zypp/ResStatus.h"
#include "zypp/VendorAttr.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/ZConfig.h"
#include "zypp/sat/Pool.h"
#include "zypp/sat/WhatProvides.h"
#include "zypp/solver/detail/SATResolver.h"
#include "zypp/solver/detail/ProblemSolutionCombi.h"
#include "zypp/solver/detail/ProblemSolutionIgnore.h"
#include "zypp/solver/detail/SolverQueueItemInstall.h"
#include "zypp/solver/detail/SolverQueueItemDelete.h"
#include "zypp/solver/detail/SystemCheck.h"

extern "C" {
#include "satsolver/repo_solv.h"
#include "satsolver/poolarch.h"
#include "satsolver/evr.h"
#include "satsolver/poolvendor.h"
#include "satsolver/policy.h"
#include "satsolver/bitmap.h"
#include "satsolver/queue.h"
}

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

IMPL_PTR_TYPE(SATResolver);

//---------------------------------------------------------------------------
// Callbacks for SAT policies
//---------------------------------------------------------------------------

int vendorCheck (Pool *pool, Solvable *solvable1, Solvable *solvable2) {
//    DBG << "vendorCheck: " << id2str(pool, solvable1->vendor) << " <--> " << id2str(pool, solvable1->vendor) << endl;
    return VendorAttr::instance().equivalent(id2str(pool, solvable1->vendor), id2str(pool, solvable2->vendor)) ? 0:1;
}


string
itemToString (PoolItem item, bool shortVersion)
{
    ostringstream os;
    if (!item) return "";

    if (item->kind() != ResKind::package)
	os << item->kind() << ':';
    os  << item->name();
    if (!shortVersion) {
	os << '-' << item->edition();
	if (item->arch() != "") {
	    os << '.' << item->arch();
	}
	Repository s = item->repository();
	if (s) {
	    string alias = s.info().alias();
	    if (!alias.empty()
		&& alias != "@system")
	    {
		os << '[' << s.info().alias() << ']';
	    }
	}
    }
    return os.str();
}


//---------------------------------------------------------------------------

std::ostream &
SATResolver::dumpOn( std::ostream & os ) const
{
    os << "<resolver>" << endl;
    if (_solv) {
	os << "  fixsystem = " << _solv->fixsystem << endl;
	os << "  allowdowngrade = " << _solv->allowdowngrade << endl;
	os << "  allowarchchange = " << _solv->allowarchchange << endl;
	os << "  allowvendorchange = " <<  _solv->allowvendorchange << endl;
	os << "  allowuninstall = " << _solv->allowuninstall << endl;
	os << "  updatesystem = " << _solv->updatesystem << endl;
	os << "  allowvirtualconflicts = " <<  _solv->allowvirtualconflicts << endl;
	os << "  noupdateprovide = " << _solv->noupdateprovide << endl;
	os << "  dosplitprovides = " << _solv->dosplitprovides << endl;
	os << "  onlyRequires = " << _solv->dontinstallrecommended << endl;
	os << "  ignorealreadyrecommended = " << _solv->ignorealreadyrecommended << endl;
	os << "  distupgrade = " << _distupgrade << endl;
        os << "  distupgrade_removeunsupported = " << _distupgrade_removeunsupported << endl;

    } else {
	os << "<NULL>";
    }
    return os << "<resolver/>" << endl;
}

//---------------------------------------------------------------------------

SATResolver::SATResolver (const ResPool & pool, Pool *SATPool)
    : _pool (pool)
    , _SATPool (SATPool)
    , _solv(NULL)
    , _fixsystem(false)
    , _allowdowngrade(false)
    , _allowarchchange(false)
    , _allowvendorchange(false)
    , _allowuninstall(false)
    , _updatesystem(false)
    , _allowvirtualconflicts(false)
    , _noupdateprovide(false)
    , _dosplitprovides(false)
    , _onlyRequires(ZConfig::instance().solver_onlyRequires())
    , _ignorealreadyrecommended(false)
    , _distupgrade(false)
    , _distupgrade_removeunsupported(false)

{
}


SATResolver::~SATResolver()
{
}

//---------------------------------------------------------------------------


ResPool
SATResolver::pool (void) const
{
    return _pool;
}


void
SATResolver::resetItemTransaction (PoolItem item)
{
    bool found = false;
    for (PoolItemList::const_iterator iter = _items_to_remove.begin();
	 iter != _items_to_remove.end(); iter++) {
	if (*iter == item) {
	    _items_to_remove.remove(*iter);
	    found = true;
	    break;
	}
    }
    if (!found) {
	for (PoolItemList::const_iterator iter = _items_to_install.begin();
	     iter != _items_to_install.end(); iter++) {
	    if (*iter == item) {
		_items_to_install.remove(*iter);
		found = true;
		break;
	    }
	}
    }
    if (!found) {
	for (PoolItemList::const_iterator iter = _items_to_keep.begin();
	     iter != _items_to_keep.end(); iter++) {
	    if (*iter == item) {
		_items_to_keep.remove(*iter);
		found = true;
		break;
	    }
	}
    }
    if (!found) {
	for (PoolItemList::const_iterator iter = _items_to_lock.begin();
	     iter != _items_to_lock.end(); iter++) {
	    if (*iter == item) {
		_items_to_lock.remove(*iter);
		found = true;
		break;
	    }
	}
    }
}


void
SATResolver::addPoolItemToInstall (PoolItem item)
{
    resetItemTransaction (item);
    _items_to_install.push_back (item);
    _items_to_install.unique ();
}


void
SATResolver::addPoolItemsToInstallFromList (PoolItemList & rl)
{
    for (PoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToInstall (*iter);
    }
}


void
SATResolver::addPoolItemToRemove (PoolItem item)
{
    resetItemTransaction (item);
    _items_to_remove.push_back (item);
    _items_to_remove.unique ();
}


void
SATResolver::addPoolItemsToRemoveFromList (PoolItemList & rl)
{
    for (PoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToRemove (*iter);
    }
}

void
SATResolver::addPoolItemToLock (PoolItem item)
{
    resetItemTransaction (item);
    _items_to_lock.push_back (item);
    _items_to_lock.unique ();
}

void
SATResolver::addPoolItemToKeep (PoolItem item)
{
    resetItemTransaction (item);
    _items_to_keep.push_back (item);
    _items_to_keep.unique ();
}

//---------------------------------------------------------------------------

// copy marked item from solution back to pool
// if data != NULL, set as APPL_LOW (from establishPool())

static void
SATSolutionToPool (PoolItem item, const ResStatus & status, const ResStatus::TransactByValue causer)
{
    // resetting
    item.status().resetTransact (causer);
    item.status().resetWeak ();

    bool r;

    // installation/deletion
    if (status.isToBeInstalled()) {
	r = item.status().setToBeInstalled (causer);
	_XDEBUG("SATSolutionToPool(" << item << ", " << status << ") install !" << r);
    }
    else if (status.isToBeUninstalledDueToUpgrade()) {
	r = item.status().setToBeUninstalledDueToUpgrade (causer);
	_XDEBUG("SATSolutionToPool(" << item << ", " << status << ") upgrade !" << r);
    }
    else if (status.isToBeUninstalled()) {
	r = item.status().setToBeUninstalled (causer);
	_XDEBUG("SATSolutionToPool(" << item << ", " << status << ") remove !" << r);
    }

    return;
}


//----------------------------------------------------------------------------
// helper functions for distupgrade
//----------------------------------------------------------------------------

bool SATResolver::doesObsoleteItem (PoolItem candidate, PoolItem installed) {
  Solvable *sCandidate = _SATPool->solvables + candidate.satSolvable().id();
  ::_Repo *installedRepo = sat::Pool::instance().systemRepo().get();

  Id p, pp, obsolete, *obsoleteIt;

  if ((!installedRepo || sCandidate->repo != installedRepo) && sCandidate->obsoletes) {
      obsoleteIt = sCandidate->repo->idarraydata + sCandidate->obsoletes;
      while ((obsolete = *obsoleteIt++) != 0)
      {
	  for (pp = pool_whatprovides(_SATPool, obsolete); (p = _SATPool->whatprovidesdata[pp++]) != 0; ) {
	      if (p > 0 &&  installed.satSolvable().id() == (sat::detail::SolvableIdType)p) {
		  MIL << candidate << " obsoletes " << installed << endl;
		  return true;
	      }
	  }
      }
  }
  return false;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// resolvePool
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Helper functions for the ZYPP-Pool
//----------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
//  This function loops over the pool and grabs all items
//  It clears all previous bySolver() states also
//
//  Every toBeInstalled is passed to zypp::solver:detail::Resolver.addPoolItemToInstall()
//  Every toBeUninstalled is passed to zypp::solver:detail::Resolver.addPoolItemToRemove()
//
//  Solver results must be written back to the pool.
//------------------------------------------------------------------------------------------------------------


struct SATCollectTransact : public resfilter::PoolItemFilterFunctor
{
    SATResolver & resolver;

    SATCollectTransact (SATResolver & r)
	: resolver (r)
    { }

    bool operator()( PoolItem item )		// only transacts() items go here
    {
	ResStatus status = item.status();
	bool by_solver = (status.isBySolver() || status.isByApplLow());

	if (by_solver) {
	    _XDEBUG("Resetting " << item );
	    item.status().resetTransact( ResStatus::APPL_LOW );// clear any solver/establish transactions
	    return true;				// back out here, dont re-queue former solver result
	}

	if (status.isToBeInstalled()) {
	    resolver.addPoolItemToInstall(item);	// -> install!
	}
	else if (status.isToBeUninstalled()) {
	    resolver.addPoolItemToRemove(item);		// -> remove !
	}
        else if (status.isLocked()
		 && !by_solver) {
	    resolver.addPoolItemToLock (item);
        }
        else if (status.isKept()
		 && !by_solver) {
	    resolver.addPoolItemToKeep (item);
        }

	return true;
    }
};


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// solving.....
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


class CheckIfUpdate : public resfilter::PoolItemFilterFunctor
{
  public:
    bool is_updated;

    CheckIfUpdate()
	: is_updated( false )
    {}

    // check this item will be installed

    bool operator()( PoolItem item )
    {
	if (item.status().isToBeInstalled())
	{
	    is_updated = true;
	    return false;
	}
	return true;
    }
};


class CollectNonePackages : public resfilter::PoolItemFilterFunctor
{
  public:
    Queue *solvableQueue;

    CollectNonePackages( Queue *queue )
	:solvableQueue (queue)
    {}

    // collecting none packges

    bool operator()( PoolItem item )
    {
	queue_push(solvableQueue, item.satSolvable().id());
	return true;
    }
};

bool
SATResolver::solving()
{
    _solv = solver_create( _SATPool, sat::Pool::instance().systemRepo().get() );
    _solv->vendorCheckCb = &vendorCheck;
    _solv->fixsystem = _fixsystem;
    _solv->ignorealreadyrecommended = _ignorealreadyrecommended;
    _solv->updatesystem = _updatesystem;
    _solv->allowdowngrade = _allowdowngrade;
    _solv->allowuninstall = _allowuninstall;
    _solv->allowarchchange = _allowarchchange;
    _solv->allowvendorchange = _allowvendorchange;
    _solv->allowvirtualconflicts = _allowvirtualconflicts;
    _solv->dosplitprovides = _dosplitprovides;
    _solv->noupdateprovide = _noupdateprovide;
    _solv->dontinstallrecommended = _onlyRequires;
    _solv->distupgrade = _distupgrade;
    _solv->distupgrade_removeunsupported = _distupgrade_removeunsupported;

    sat::Pool::instance().prepare();

    // Solve !
    MIL << "Starting solving...." << endl;
    MIL << *this;
    solver_solve( _solv, &(_jobQueue) );
    MIL << "....Solver end" << endl;

    // copying solution back to zypp pool
    //-----------------------------------------
    _result_items_to_install.clear();
    _result_items_to_remove.clear();

    /*  solvables to be installed */
    for (int i = 0; i < _solv->decisionq.count; i++)
    {
      Id p;
      p = _solv->decisionq.elements[i];
      if (p < 0 || !sat::Solvable(p))
	continue;
      if (sat::Solvable(p).repository().get() == _solv->installed)
	continue;

      PoolItem poolItem = _pool.find (sat::Solvable(p));
      if (poolItem) {
	  SATSolutionToPool (poolItem, ResStatus::toBeInstalled, ResStatus::SOLVER);
	  _result_items_to_install.push_back (poolItem);
      } else {
	  ERR << "id " << p << " not found in ZYPP pool." << endl;
      }
    }

    /* solvables to be erased */
    for (int i = _solv->installed->start; i < _solv->installed->start + _solv->installed->nsolvables; i++)
    {
      if (_solv->decisionmap[i] > 0)
	continue;

      PoolItem poolItem = _pool.find (sat::Solvable(i));
      if (poolItem) {
	  // Check if this is an update
	  CheckIfUpdate info;
	  invokeOnEach( _pool.byIdentBegin( poolItem ),
			_pool.byIdentEnd( poolItem ),
			resfilter::ByUninstalled(),			// ByUninstalled
			functor::functorRef<bool,PoolItem> (info) );

	  if (info.is_updated) {
	      SATSolutionToPool (poolItem, ResStatus::toBeUninstalledDueToUpgrade , ResStatus::SOLVER);
	  } else {
	      SATSolutionToPool (poolItem, ResStatus::toBeUninstalled, ResStatus::SOLVER);
	  }
	  _result_items_to_remove.push_back (poolItem);
      } else {
	  ERR << "id " << i << " not found in ZYPP pool." << endl;
      }
    }

    /*  solvables which are recommended */
    for (int i = 0; i < _solv->recommendations.count; i++)
    {
      Id p;
      p = _solv->recommendations.elements[i];
      if (p < 0 || !sat::Solvable(p))
	continue;

      PoolItem poolItem = _pool.find (sat::Solvable(p));
      if (poolItem) {
	  poolItem.status().setRecommended(true);
	  _XDEBUG("SATSolutionToPool(" << poolItem << ") recommended !");
      } else {
	  ERR << "id " << p << " not found in ZYPP pool." << endl;
      }
    }

    /*  solvables which are suggested */
    for (int i = 0; i < _solv->suggestions.count; i++)
    {
      Id p;
      p = _solv->suggestions.elements[i];
      if (p < 0 || !sat::Solvable(p))
	continue;

      PoolItem poolItem = _pool.find (sat::Solvable(p));
      if (poolItem) {
	  poolItem.status().setSuggested(true);
	  _XDEBUG("SATSolutionToPool(" << poolItem << ") suggested !");
      } else {
	  ERR << "id " << p << " not found in ZYPP pool." << endl;
      }
    }

    /* Write validation state back to pool */
    Map installedmap;
    Queue flags, solvableQueue;

    queue_init(&flags);
    queue_init(&solvableQueue);

    CollectNonePackages collectNonePackages(&solvableQueue);
    invokeOnEach( _pool.begin(),
		  _pool.end(),
		  functor::not_c(resfilter::byKind<Package>()), // every solvable BUT packages
		  functor::functorRef<bool,PoolItem> (collectNonePackages) );
    solver_create_state_maps(_solv, &installedmap, 0);
    pool_trivial_installable(_solv->pool, _solv->installed, &installedmap, &solvableQueue, &flags);
    for (int i = 0; i < solvableQueue.count; i++) {
	PoolItem item = _pool.find (sat::Solvable(solvableQueue.elements[i]));
	item.status().setUndetermined();

	if (flags.elements[i] == -1) {
	    item.status().setNonRelevant();
	    _XDEBUG("SATSolutionToPool(" << item << " ) nonRelevant !");
	} else if (flags.elements[i] == 1) {
	    item.status().setSatisfied();
	    _XDEBUG("SATSolutionToPool(" << item << " ) satisfied !");
	} else if (flags.elements[i] == 0) {
	    item.status().setBroken();
	    _XDEBUG("SATSolutionToPool(" << item << " ) broken !");
	}
    }

    if (_solv->problems.count > 0 )
    {
	ERR << "Solverrun finished with an ERROR" << endl;
	return false;
    }

    map_free(&installedmap);
    queue_free(&(solvableQueue));
    queue_free(&flags);

    return true;
}


void
SATResolver::solverInit(const PoolItemList & weakItems)
{
    SATCollectTransact info (*this);

    MIL << "SATResolver::solverInit()" << endl;

    if (_solv) {
	// remove old stuff
	solver_free(_solv);
	_solv = NULL;
	queue_free( &(_jobQueue) );
    }

    queue_init( &_jobQueue );
    _items_to_install.clear();
    _items_to_remove.clear();
    _items_to_lock.clear();
    _items_to_keep.clear();

    invokeOnEach ( _pool.begin(), _pool.end(),
		   functor::functorRef<bool,PoolItem>(info) );

    for (PoolItemList::const_iterator iter = weakItems.begin(); iter != weakItems.end(); iter++) {
	Id id = (*iter)->satSolvable().id();
	if (id == ID_NULL) {
	    ERR << "Weaken: " << *iter << " not found" << endl;
	}
	MIL << "Weaken dependencies of " << *iter << endl;
	queue_push( &(_jobQueue), SOLVER_WEAKEN_SOLVABLE_DEPS );
        queue_push( &(_jobQueue), id );
    }

    // Add rules for parallel installable resolvables with different versions
    std::set<IdString> parallel = ZConfig::instance().multiversion();
    for (std::set<IdString>::const_iterator it = parallel.begin(); it != parallel.end(); ++it) {
	queue_push( &(_jobQueue), SOLVER_NOOBSOLETES_SOLVABLE_NAME );
	queue_push( &(_jobQueue), it->id() );
    }
}

void
SATResolver::solverEnd()
{
    // cleanup
    solver_free(_solv);
    _solv = NULL;
    queue_free( &(_jobQueue) );
}


bool
SATResolver::resolvePool(const CapabilitySet & requires_caps,
			 const CapabilitySet & conflict_caps,
			 const PoolItemList & weakItems)
{
    MIL << "SATResolver::resolvePool()" << endl;

    // initialize
    solverInit(weakItems);

    for (PoolItemList::const_iterator iter = _items_to_install.begin(); iter != _items_to_install.end(); iter++) {
	Id id = (*iter)->satSolvable().id();
	if (id == ID_NULL) {
	    ERR << "Install: " << *iter << " not found" << endl;
	} else {
	    MIL << "Install " << *iter << endl;
	    queue_push( &(_jobQueue), SOLVER_INSTALL_SOLVABLE );
	    queue_push( &(_jobQueue), id );
	}
    }

    for (PoolItemList::const_iterator iter = _items_to_remove.begin(); iter != _items_to_remove.end(); iter++) {
	Id id = (*iter)->satSolvable().id();
	if (id == ID_NULL) {
	    ERR << "Delete: " << *iter << " not found" << endl;
	} else {
	    MIL << "Delete " << *iter << endl;
	    queue_push( &(_jobQueue), SOLVER_ERASE_SOLVABLE );
	    queue_push( &(_jobQueue), id);
	}
    }

    for (CapabilitySet::const_iterator iter = requires_caps.begin(); iter != requires_caps.end(); iter++) {
	queue_push( &(_jobQueue), SOLVER_INSTALL_SOLVABLE_PROVIDES );
	queue_push( &(_jobQueue), iter->id() );
	MIL << "Requires " << *iter << endl;
    }

    for (CapabilitySet::const_iterator iter = conflict_caps.begin(); iter != conflict_caps.end(); iter++) {
	queue_push( &(_jobQueue), SOLVER_ERASE_SOLVABLE_PROVIDES);
	queue_push( &(_jobQueue), iter->id() );
	MIL << "Conflicts " << *iter << endl;
    }

    // set requirements for a running system
    setSystemRequirements();

    // set locks for the solver
    setLocks();

    // solving
    bool ret = solving();
    // cleanup
    if (ret)
	solverEnd(); // remove solver only if no errors happend. Need it for solving problems

    MIL << "SATResolver::resolvePool() done. Ret:" << ret <<  endl;
    return ret;
}


bool
SATResolver::resolveQueue(const SolverQueueItemList &requestQueue,
			  const PoolItemList & weakItems)
{
    MIL << "SATResolver::resolvQueue()" << endl;

    // initialize
    solverInit(weakItems);

    // generate solver queue
    for (SolverQueueItemList::const_iterator iter = requestQueue.begin(); iter != requestQueue.end(); iter++) {
	(*iter)->addRule(_jobQueue);
    }

    // Add addition item status to the resolve-queue cause these can be set by problem resolutions
    for (PoolItemList::const_iterator iter = _items_to_install.begin(); iter != _items_to_install.end(); iter++) {
	Id id = (*iter)->satSolvable().id();
	if (id == ID_NULL) {
	    ERR << "Install: " << *iter << " not found" << endl;
	} else {
	    MIL << "Install " << *iter << endl;
	    queue_push( &(_jobQueue), SOLVER_INSTALL_SOLVABLE );
	    queue_push( &(_jobQueue), id );
	}
    }
    for (PoolItemList::const_iterator iter = _items_to_remove.begin(); iter != _items_to_remove.end(); iter++) {
        sat::detail::IdType ident( (*iter)->satSolvable().ident().id() );
	MIL << "Delete " << *iter << ident << endl;
	queue_push( &(_jobQueue), SOLVER_ERASE_SOLVABLE_NAME );
	queue_push( &(_jobQueue), ident);
    }

    // set requirements for a running system
    setSystemRequirements();

    // set locks for the solver
    setLocks();

    // solving
    bool ret = solving();

    // cleanup
    if (ret)
	solverEnd(); // remove solver only if no errors happend. Need it for solving problems

    MIL << "SATResolver::resolveQueue() done. Ret:" << ret <<  endl;
    return ret;
}


void SATResolver::doUpdate()
{
    MIL << "SATResolver::doUpdate()" << endl;

    // initialize
    solverInit(PoolItemList());

    // set requirements for a running system
    setSystemRequirements();

    // set locks for the solver
    void setLocks();

    _solv = solver_create( _SATPool, sat::Pool::instance().systemRepo().get() );
    _solv->vendorCheckCb = &vendorCheck;

    _solv->updatesystem = true;
    _solv->dontinstallrecommended = true; // #FIXME dontinstallrecommended maybe set to false if it works correctly

    sat::Pool::instance().prepare();

    // Solve !
    MIL << "Starting solving for update...." << endl;
    MIL << *this;
    solver_solve( _solv, &(_jobQueue) );
    MIL << "....Solver end" << endl;

    // copying solution back to zypp pool
    //-----------------------------------------

    /*  solvables to be installed */
    for (int i = 0; i < _solv->decisionq.count; i++)
    {
      Id p;
      p = _solv->decisionq.elements[i];
      if (p < 0 || !sat::Solvable(p))
	continue;
      if (sat::Solvable(p).repository().get() == _solv->installed)
	continue;

      PoolItem poolItem = _pool.find (sat::Solvable(p));
      if (poolItem) {
	  SATSolutionToPool (poolItem, ResStatus::toBeInstalled, ResStatus::SOLVER);
      } else {
	  ERR << "id " << p << " not found in ZYPP pool." << endl;
      }
    }

    /* solvables to be erased */
    for (int i = _solv->installed->start; i < _solv->installed->start + _solv->installed->nsolvables; i++)
    {
      if (_solv->decisionmap[i] > 0)
	continue;

      PoolItem poolItem = _pool.find (sat::Solvable(i));
      if (poolItem) {
	  // Check if this is an update
	  CheckIfUpdate info;
	  invokeOnEach( _pool.byIdentBegin( poolItem ),
			_pool.byIdentEnd( poolItem ),
			resfilter::ByUninstalled(),			// ByUninstalled
			functor::functorRef<bool,PoolItem> (info) );

	  if (info.is_updated) {
	      SATSolutionToPool (poolItem, ResStatus::toBeUninstalledDueToUpgrade , ResStatus::SOLVER);
	  } else {
	      SATSolutionToPool (poolItem, ResStatus::toBeUninstalled, ResStatus::SOLVER);
	  }
      } else {
	  ERR << "id " << i << " not found in ZYPP pool." << endl;
      }
    }

    // cleanup
    solverEnd();

    MIL << "SATResolver::doUpdate() done" << endl;
}



//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// error handling
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// helper function
//----------------------------------------------------------------------------

struct FindPackage : public resfilter::ResObjectFilterFunctor
{
    ProblemSolutionCombi *problemSolution;
    TransactionKind action;
    FindPackage (ProblemSolutionCombi *p, const TransactionKind act)
       : problemSolution (p)
       , action (act)
	{
	}

    bool operator()( PoolItem p)
   {
       problemSolution->addSingleAction (p, action);
       return true;
   }
};


//----------------------------------------------------------------------------
// Checking if this solvable/item has a buddy which reflect the real
// user visible description of an item
// e.g. The release package has a buddy to the concerning product item.
// This user want's the message "Product foo conflicts with product bar" and
// NOT "package release-foo conflicts with package release-bar"
//----------------------------------------------------------------------------


PoolItem SATResolver::mapItem (const PoolItem &item)
{
    sat::Solvable buddy = item.buddy();
    if (buddy != sat::Solvable())
    {
	return _pool.find (buddy);
    }
    else
    {
	return item;
    }
}

sat::Solvable SATResolver::mapSolvable (const Id &id)
{
    PoolItem item = _pool.find (sat::Solvable(id));
    return mapItem(item).satSolvable();
}

string SATResolver::SATprobleminfoString(Id problem, string &detail, Id &ignoreId)
{
  string ret;
  Pool *pool = _solv->pool;
  Id probr;
  Id dep, source, target;
  sat::Solvable s, s2;

  ignoreId = 0;
  probr = solver_findproblemrule(_solv, problem);
  switch (solver_problemruleinfo(_solv, &(_jobQueue), probr, &dep, &source, &target))
  {
      case SOLVER_PROBLEM_UPDATE_RULE:
	  s = mapSolvable (source);
	  ret = str::form (_("problem with installed package %s"), solvable2str(pool, s.get()));
	  break;
      case SOLVER_PROBLEM_JOB_RULE:
	  ret = str::form (_("conflicting requests"));
	  break;
      case SOLVER_PROBLEM_RPM_RULE:
	  ret = str::form (_("some dependency problem"));
	  break;
      case SOLVER_PROBLEM_JOB_NOTHING_PROVIDES_DEP:
	  ret = str::form (_("nothing provides requested %s"), dep2str(pool, dep));
	  detail += _("Have you enabled all requested repositories?");
	  break;
      case SOLVER_PROBLEM_NOT_INSTALLABLE:
	  s = mapSolvable (source);
	  ret = str::form (_("%s is not installable"), solvable2str(pool, s.get()));
	  break;
      case SOLVER_PROBLEM_NOTHING_PROVIDES_DEP:
	  s = mapSolvable (source);
	  ret = str::form (_("nothing provides %s needed by %s"), dep2str(pool, dep), solvable2str(pool, s.get()));
	  break;
      case SOLVER_PROBLEM_SAME_NAME:
	  s = mapSolvable (source);
	  s2 = mapSolvable (target);
	  ret = str::form (_("cannot install both %s and %s"), solvable2str(pool, s.get()), solvable2str(pool, s2.get()));
	  break;
      case SOLVER_PROBLEM_PACKAGE_CONFLICT:
	  s = mapSolvable (source);
	  s2 = mapSolvable (target);
	  ret = str::form (_("%s conflicts with %s provided by %s"), solvable2str(pool, s.get()), dep2str(pool, dep), solvable2str(pool, s2.get()));
	  break;
      case SOLVER_PROBLEM_PACKAGE_OBSOLETES:
	  s = mapSolvable (source);
	  s2 = mapSolvable (target);
	  ret = str::form (_("%s obsoletes %s provided by %s"), solvable2str(pool, s.get()), dep2str(pool, dep), solvable2str(pool, s2.get()));
	  break;
      case SOLVER_PROBLEM_SELF_CONFLICT:
	  s = mapSolvable (source);
	  ret = str::form (_("Solvable %s conflicts with %s provided by itself"), solvable2str(pool, s.get()), dep2str(pool, dep));
          break;
      case SOLVER_PROBLEM_DEP_PROVIDERS_NOT_INSTALLABLE:
	  ignoreId = source; // for setting weak dependencies
	  s = mapSolvable (source);
	  Capability cap(dep);
	  sat::WhatProvides possibleProviders(cap);

	  // check, if a provider will be deleted
	  typedef list<PoolItem> ProviderList;
	  ProviderList providerlistInstalled, providerlistUninstalled;
	  for_( iter1, possibleProviders.begin(), possibleProviders.end() ) {
	      PoolItem provider1 = ResPool::instance().find( *iter1 );
	      // find pair of an installed/uninstalled item with the same NVR
	      bool found = false;
	      for_( iter2, possibleProviders.begin(), possibleProviders.end() ) {
		  PoolItem provider2 = ResPool::instance().find( *iter2 );
		  if (compareByNVR (provider1.resolvable(),provider2.resolvable()) == 0
		      && ( (provider1.status().isInstalled() && provider2.status().isUninstalled())
			  || (provider2.status().isInstalled() && provider1.status().isUninstalled()) ))  {
		      found = true;
		      break;
		  }
	      }
	      if (!found) {
		  if (provider1.status().isInstalled())
		      providerlistInstalled.push_back(provider1);
		  else
		      providerlistUninstalled.push_back(provider1);
	      }
	  }

	  ret = str::form (_("%s requires %s, but this requirement cannot be provided"), solvable2str(pool, s.get()), dep2str(pool, dep));
	  if (providerlistInstalled.size() > 0) {
	      detail += _("deleted providers: ");
	      for (ProviderList::const_iterator iter = providerlistInstalled.begin(); iter != providerlistInstalled.end(); iter++) {
		  if (iter == providerlistInstalled.begin())
		      detail += itemToString (*iter, false);
		  else
		      detail += "\n                   " + itemToString (mapItem(*iter), false);
	      }
	  }
	  if (providerlistUninstalled.size() > 0) {
	      if (detail.size() > 0)
		  detail += _("\nuninstallable providers: ");
	      else
		  detail = _("uninstallable providers: ");
	      for (ProviderList::const_iterator iter = providerlistUninstalled.begin(); iter != providerlistUninstalled.end(); iter++) {
		  if (iter == providerlistUninstalled.begin())
		      detail += itemToString (*iter, false);
		  else
		      detail += "\n                   " + itemToString (mapItem(*iter), false);
	      }
	  }
	  break;
  }

  return ret;
}

ResolverProblemList
SATResolver::problems ()
{
    ResolverProblemList resolverProblems;
    if (_solv && _solv->problems.count) {
	Pool *pool = _solv->pool;
	int pcnt;
	Id p, rp, what;
	Id problem, solution, element;
	sat::Solvable s, sd;

	CapabilitySet system_requires = SystemCheck::instance().requiredSystemCap();
	CapabilitySet system_conflicts = SystemCheck::instance().conflictSystemCap();

	MIL << "Encountered problems! Here are the solutions:\n" << endl;
	pcnt = 1;
	problem = 0;
	while ((problem = solver_next_problem(_solv, problem)) != 0) {
	    MIL << "Problem " <<  pcnt++ << ":" << endl;
	    MIL << "====================================" << endl;
	    string detail;
	    Id ignoreId;
	    string whatString = SATprobleminfoString (problem,detail,ignoreId);
	    MIL << whatString << endl;
	    MIL << "------------------------------------" << endl;
	    ResolverProblem_Ptr resolverProblem = new ResolverProblem (whatString, detail);

	    solution = 0;
	    while ((solution = solver_next_solution(_solv, problem, solution)) != 0) {
		element = 0;
		ProblemSolutionCombi *problemSolution = new ProblemSolutionCombi(resolverProblem);
		while ((element = solver_next_solutionelement(_solv, problem, solution, element, &p, &rp)) != 0) {
		    if (p == 0) {
			/* job, rp is index into job queue */
			what = _jobQueue.elements[rp];
			switch (_jobQueue.elements[rp-1])
			{
			    case SOLVER_INSTALL_SOLVABLE: {
				s = mapSolvable (what);
				PoolItem poolItem = _pool.find (s);
				if (poolItem) {
				    if (_solv->installed && s.get()->repo == _solv->installed) {
					problemSolution->addSingleAction (poolItem, REMOVE);
					string description = str::form (_("do not keep %s installed"),  solvable2str(pool, s.get()) );
					MIL << description << endl;
					problemSolution->addDescription (description);
				    } else {
					problemSolution->addSingleAction (poolItem, REMOVE);
					string description = str::form (_("do not install %s"), solvable2str(pool, s.get()));
					MIL << description << endl;
					problemSolution->addDescription (description);
				    }
				} else {
				    ERR << "SOLVER_INSTALL_SOLVABLE: No item found for " << id2str(pool, s.get()->name) << "-"
					<<  id2str(pool, s.get()->evr) << "." <<  id2str(pool, s.get()->arch) << endl;
				}
			    }
				break;
			    case SOLVER_ERASE_SOLVABLE: {
				s = mapSolvable (what);
				PoolItem poolItem = _pool.find (s);
				if (poolItem) {
				    if (_solv->installed && s.get()->repo == _solv->installed) {
					problemSolution->addSingleAction (poolItem, KEEP);
					string description = str::form (_("keep %s"), solvable2str(pool, s.get()));
					MIL << description << endl;
					problemSolution->addDescription (description);
				    } else {
					problemSolution->addSingleAction (poolItem, INSTALL);
					string description = str::form (_("do not forbid installation of %s"), solvable2str(pool, s.get()));
					MIL << description << endl;
					problemSolution->addDescription (description);
				    }
				} else {
				    ERR << "SOLVER_ERASE_SOLVABLE: No item found for " << id2str(pool, s.get()->name) << "-" <<  id2str(pool, s.get()->evr) << "." <<
					id2str(pool, s.get()->arch) << endl;
				}
			    }
				break;
			    case SOLVER_INSTALL_SOLVABLE_NAME:
				{
				IdString ident( what );
				SolverQueueItemInstall_Ptr install =
				    new SolverQueueItemInstall(_pool, ident.asString(), false );
				problemSolution->addSingleAction (install, REMOVE_SOLVE_QUEUE_ITEM);

				string description = str::form (_("do not install %s"), ident.c_str() );
				MIL << description << endl;
				problemSolution->addDescription (description);
				}
				break;
			    case SOLVER_ERASE_SOLVABLE_NAME:
				{
				// As we do not know, if this request has come from resolvePool or
				// resolveQueue we will have to take care for both cases.
                                IdString ident( what );
				FindPackage info (problemSolution, KEEP);
				invokeOnEach( _pool.byIdentBegin( ident ),
					      _pool.byIdentEnd( ident ),
					      functor::chain (resfilter::ByInstalled (),			// ByInstalled
							      resfilter::ByTransact ()),			// will be deinstalled
					      functor::functorRef<bool,PoolItem> (info) );

				SolverQueueItemDelete_Ptr del =
				    new SolverQueueItemDelete(_pool, ident.asString(), false );
				problemSolution->addSingleAction (del, REMOVE_SOLVE_QUEUE_ITEM);

				string description = str::form (_("keep %s"), ident.c_str());
				MIL << description << endl;
				problemSolution->addDescription (description);
				}
				break;
			    case SOLVER_INSTALL_SOLVABLE_PROVIDES:
				{
				problemSolution->addSingleAction (Capability(what), REMOVE_EXTRA_REQUIRE);
				string description = "";

				// Checking if this problem solution would break your system
				if (system_requires.find(Capability(what)) != system_requires.end()) {
				    // Show a better warning
				    resolverProblem->setDetails( resolverProblem->description() + "\n" + resolverProblem->details() );
				    resolverProblem->setDescription(_("This request will break your system!"));
				    description = _("ignore the warning of a broken system");
				} else {
				    description = str::form (_("do not ask to install a solvable providing %s"), dep2str(pool, what));
				}
				MIL << description << endl;
				problemSolution->addDescription (description);
				}
				break;
			    case SOLVER_ERASE_SOLVABLE_PROVIDES:
				{
				problemSolution->addSingleAction (Capability(what), REMOVE_EXTRA_CONFLICT);
				string description = "";

				// Checking if this problem solution would break your system
				if (system_conflicts.find(Capability(what)) != system_conflicts.end()) {
				    // Show a better warning
				    resolverProblem->setDetails( resolverProblem->description() + "\n" + resolverProblem->details() );
				    resolverProblem->setDescription(_("This request will break your system!"));
				    description = _("ignore the warning of a broken system");
				} else {
				    description = str::form (_("do not ask to delete all solvables providing %s"), dep2str(pool, what));
				}
				MIL << description << endl;
				problemSolution->addDescription (description);
				}
				break;
			    case SOLVER_INSTALL_SOLVABLE_UPDATE:
				{
				s = mapSolvable (what);
				PoolItem poolItem = _pool.find (s);
				if (poolItem) {
				    if (_solv->installed && s.get()->repo == _solv->installed) {
					problemSolution->addSingleAction (poolItem, KEEP);
					string description = str::form (_("do not install most recent version of %s"), solvable2str(pool, s.get()));
					MIL << description << endl;
					problemSolution->addDescription (description);
				    } else {
					ERR << "SOLVER_INSTALL_SOLVABLE_UPDATE " << poolItem << " is not selected for installation" << endl;
				    }
				} else {
				    ERR << "SOLVER_INSTALL_SOLVABLE_UPDATE: No item found for " << id2str(pool, s.get()->name) << "-" <<  id2str(pool, s.get()->evr) << "." <<
					id2str(pool, s.get()->arch) << endl;
				}
				}
				break;
			    default:
				MIL << "- do something different" << endl;
				ERR << "No valid solution available" << endl;
				break;
			}
		    } else {
			/* policy, replace p with rp */
			s = mapSolvable (p);
			if (rp)
			    sd = mapSolvable (rp);

			PoolItem itemFrom = _pool.find (s);
			if (rp)
			{
			    int gotone = 0;

			    PoolItem itemTo = _pool.find (sd);
			    if (itemFrom && itemTo) {
				problemSolution->addSingleAction (itemTo, INSTALL);

				if (evrcmp(pool, s.get()->evr, sd.get()->evr, EVRCMP_COMPARE ) > 0)
				{
				    string description = str::form (_("downgrade of %s to %s"), solvable2str(pool, s.get()), solvable2str(pool, sd.get()));
				    MIL << description << endl;
				    problemSolution->addDescription (description);
				    gotone = 1;
				}
				if (!_solv->allowarchchange && s.get()->name == sd.get()->name && s.get()->arch != sd.get()->arch
				    && policy_illegal_archchange(_solv, s.get(), sd.get()))
				{
				    string description = str::form (_("architecture change of %s to %s"), solvable2str(pool, s.get()), solvable2str(pool, sd.get()));
				    MIL << description << endl;
				    problemSolution->addDescription (description);
				    gotone = 1;
				}
				if (!_solv->allowvendorchange && s.get()->name == sd.get()->name && s.get()->vendor != sd.get()->vendor
				    && policy_illegal_vendorchange(_solv, s.get(), sd.get()))
				{
				    string description = str::form (_("install %s (with vendor change)\n  %s\n-->\n  %s") ,
								    solvable2str(pool, sd.get()) , id2str(pool, s.get()->vendor),
								    string(sd.get()->vendor ?  id2str(pool, sd.get()->vendor) : " (no vendor) ").c_str() );
				    MIL << description << endl;
				    problemSolution->addDescription (description);
				    gotone = 1;
				}
				if (!gotone) {
				    string description = str::form (_("replacement of %s with %s"), solvable2str(pool, s.get()), solvable2str(pool, sd.get()));
				    MIL << description << endl;
				    problemSolution->addDescription (description);
				}
			    } else {
				ERR << id2str(pool, s.get()->name) << "-" <<  id2str(pool, s.get()->evr) << "." <<  id2str(pool, s.get()->arch)
				    << " or "  << id2str(pool, sd.get()->name) << "-" <<  id2str(pool, sd.get()->evr) << "." <<  id2str(pool, sd.get()->arch) << " not found" << endl;
			    }
			}
			else
			{
			    if (itemFrom) {
				string description = str::form (_("deinstallation of %s"), solvable2str(pool, s.get()));
				MIL << description << endl;
				problemSolution->addDescription (description);
				problemSolution->addSingleAction (itemFrom, REMOVE);
			    }
			}
		    }
		}
		resolverProblem->addSolution (problemSolution,
					      problemSolution->actionCount() > 1 ? true : false); // Solutions with more than 1 action will be shown first.
		MIL << "------------------------------------" << endl;
	    }

	    if (ignoreId > 0) {
		// There is a possibility to ignore this error by setting weak dependencies
		PoolItem item = _pool.find (sat::Solvable(ignoreId));
		ProblemSolutionIgnore *problemSolution = new ProblemSolutionIgnore(resolverProblem, item);
		resolverProblem->addSolution (problemSolution,
					      false); // Solutions will be shown at the end
		MIL << "Ignore some dependencies of " << item << endl;
		MIL << "------------------------------------" << endl;
	    }

	    // save problem
	    resolverProblems.push_back (resolverProblem);
	}
    }
    return resolverProblems;
}

void
SATResolver::applySolutions (const ProblemSolutionList & solutions)
{
    for (ProblemSolutionList::const_iterator iter = solutions.begin();
	 iter != solutions.end(); ++iter) {
	ProblemSolution_Ptr solution = *iter;
	Resolver dummyResolver(_pool);
	if (!solution->apply (dummyResolver))
	    break;
    }
}

void SATResolver::setLocks()
{
    for (PoolItemList::const_iterator iter = _items_to_lock.begin(); iter != _items_to_lock.end(); iter++) {
        sat::detail::SolvableIdType ident( (*iter)->satSolvable().id() );
	if (iter->status().isInstalled()) {
	    MIL << "Lock installed item " << *iter << endl;
	    queue_push( &(_jobQueue), SOLVER_INSTALL_SOLVABLE );
	    queue_push( &(_jobQueue), ident );
	} else {
	    MIL << "Lock NOT installed item " << *iter << endl;
	    queue_push( &(_jobQueue), SOLVER_ERASE_SOLVABLE );
	    queue_push( &(_jobQueue), ident );
	}
    }

    for (PoolItemList::const_iterator iter = _items_to_keep.begin(); iter != _items_to_keep.end(); iter++) {
        sat::detail::SolvableIdType ident( (*iter)->satSolvable().id() );
	if (iter->status().isInstalled()) {
	    MIL << "Keep installed item " << *iter << endl;
	    queue_push( &(_jobQueue), SOLVER_INSTALL_SOLVABLE | SOLVER_WEAK);
	    queue_push( &(_jobQueue), ident );
	} else {
	    MIL << "Keep NOT installed item " << *iter << ident << endl;
	    queue_push( &(_jobQueue), SOLVER_ERASE_SOLVABLE | SOLVER_WEAK);
	    queue_push( &(_jobQueue), ident );
	}
    }
}

void SATResolver::setSystemRequirements()
{
    CapabilitySet system_requires = SystemCheck::instance().requiredSystemCap();
    CapabilitySet system_conflicts = SystemCheck::instance().conflictSystemCap();

    for (CapabilitySet::const_iterator iter = system_requires.begin(); iter != system_requires.end(); iter++) {
	queue_push( &(_jobQueue), SOLVER_INSTALL_SOLVABLE_PROVIDES );
	queue_push( &(_jobQueue), iter->id() );
	MIL << "SYSTEM Requires " << *iter << endl;
    }

    for (CapabilitySet::const_iterator iter = system_conflicts.begin(); iter != system_conflicts.end(); iter++) {
	queue_push( &(_jobQueue), SOLVER_ERASE_SOLVABLE_PROVIDES);
	queue_push( &(_jobQueue), iter->id() );
	MIL << "SYSTEM Conflicts " << *iter << endl;
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

