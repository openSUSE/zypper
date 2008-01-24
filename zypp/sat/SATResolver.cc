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

#include "zypp/solver/detail/Helper.h"
#include "zypp/base/String.h"
#include "zypp/Capability.h"
#include "zypp/ResStatus.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/sat/SATResolver.h"
#include "zypp/sat/Pool.h"
#include "zypp/solver/detail/ProblemSolutionCombi.h"

extern "C" {
#include "satsolver/repo_solv.h"
#include "satsolver/poolarch.h"
#include "satsolver/evr.h"
#include "satsolver/poolvendor.h"
#include "satsolver/policy.h"
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

static PoolItemSet triggeredSolution;   // only the latest state of an item is interesting
                                        // for the pool. Documents already inserted items.

//---------------------------------------------------------------------------


std::ostream &
SATResolver::dumpOn( std::ostream & os ) const
{
    return os << "<resolver/>";
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
SATResolver::addPoolItemToInstall (PoolItem_Ref item)
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
	_items_to_install.push_back (item);
	_items_to_install.unique ();
    }
}


void
SATResolver::addPoolItemsToInstallFromList (PoolItemList & rl)
{
    for (PoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToInstall (*iter);
    }
}


void
SATResolver::addPoolItemToRemove (PoolItem_Ref item)
{
    bool found = false;
    for (PoolItemList::const_iterator iter = _items_to_install.begin();
	 iter != _items_to_install.end(); iter++) {
	if (*iter == item) {
	    _items_to_install.remove(*iter);
	    found = true;
	    break;
	}
    }
    if (!found) {
	_items_to_remove.push_back (item);
	_items_to_remove.unique ();
    }
}


void
SATResolver::addPoolItemsToRemoveFromList (PoolItemList & rl)
{
    for (PoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToRemove (*iter);
    }
}

void
SATResolver::addPoolItemToLock (PoolItem_Ref item)
{
    _items_to_lock.push_back (item);
    _items_to_lock.unique ();
}


//---------------------------------------------------------------------------

// copy marked item from solution back to pool
// if data != NULL, set as APPL_LOW (from establishPool())

static void
SATSolutionToPool (PoolItem_Ref item, const ResStatus & status, const ResStatus::TransactByValue causer)
{
#if 0
    if (triggeredSolution.find(item) != triggeredSolution.end()) {
        _XDEBUG("SATSolutionToPool(" << item << ") is already in the pool --> skip");
        return;
    }
#endif

    triggeredSolution.insert(item);

    // resetting transaction only
    item.status().resetTransact (causer);

    bool r;

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
    else if (status.isIncomplete()
	     || status.isNeeded()) {
	r = item.status().setIncomplete();
	_XDEBUG("SATSolutionToPool(" << item << ", " << status << ") incomplete !" << r);
    }
    else if (status.isUnneeded()) {
	r = item.status().setUnneeded();
	_XDEBUG("SATSolutionToPool(" << item << ", " << status << ") unneeded !" << r);
    }
    else if (status.isSatisfied()) {
	r = item.status().setSatisfied();
	_XDEBUG("SATSolutionToPool(" << item << ", " << status << ") satisfied !" << r);
    } else {
	_XDEBUG("SATSolutionToPool(" << item << ", " << status << ") unchanged !");
    }
    return;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// resolvePool
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Helper functions for the ZYPP-Pool
//----------------------------------------------------------------------------


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
	else if (str == "atom") {
	    kind = ResTraits<zypp::Atom>::kind;
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
	else if (str == "language") {
	    kind = ResTraits<zypp::Language>::kind;
	}
	else {
	    ERR << "string2kind unknown kind '" << str << "'" << endl;
	}
    }
    return kind;
}


//------------------------------------------------------------------------------------------------------------
//  This function loops over the pool and grabs
//  all item.status().transacts() and item.status().byUser()
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

    bool operator()( PoolItem_Ref item )		// only transacts() items go here
    {
	ResStatus status = item.status();
	_XDEBUG( "SATCollectTransact(" << item << ")" );
	bool by_solver = (status.isBySolver() || status.isByApplLow());

	if (by_solver) {
	    _XDEBUG("Resetting " << item );
	    item.status().resetTransact( ResStatus::APPL_LOW );// clear any solver/establish transactions
	    return true;				// back out here, dont re-queue former solver result
	}

	if (status.isToBeInstalled()) {
	    resolver.addPoolItemToInstall(item);	// -> install!
	}
	if (status.isToBeUninstalled()) {
	    resolver.addPoolItemToRemove(item);		// -> remove !
	}
	if (status.isIncomplete()) {			// incomplete (re-install needed)
	    PoolItem_Ref reinstall = Helper::findReinstallItem (resolver.pool(), item);
	    if (reinstall) {
		MIL << "Reinstall " << reinstall << " for incomplete " << item << endl;
		resolver.addPoolItemToInstall(reinstall);	// -> install!
	    }
	    else {
		WAR << "Can't find " << item << " for re-installation" << endl;
	    }
	}

        if (status.isLocked()
            || (status.isKept()
		&& !by_solver)) {
            resolver.addPoolItemToLock (item);
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

    bool operator()( PoolItem_Ref item )
    {
	if (item.status().isToBeInstalled())
	{
	    is_updated = true;
	    return false;
	}
	return true;
    }
};


bool
SATResolver::resolvePool(const CapabilitySet & requires_caps,
			 const CapabilitySet & conflict_caps)
{
    SATCollectTransact info (*this);
    MIL << "SATResolver::resolvePool()" << endl;

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

    invokeOnEach ( _pool.begin(), _pool.end(),
		   resfilter::ByTransact( ),			// collect transacts from Pool to resolver queue
		   functor::functorRef<bool,PoolItem>(info) );

    invokeOnEach ( _pool.begin(), _pool.end(),
                   resfilter::ByLock( ),                        // collect locks from Pool to resolver queue
                   functor::functorRef<bool,PoolItem>(info) );

    invokeOnEach ( _pool.begin(), _pool.end(),
                   resfilter::ByKeep( ),                        // collect keeps from Pool to resolver queue
                   functor::functorRef<bool,PoolItem>(info) );

    for (PoolItemList::const_iterator iter = _items_to_install.begin(); iter != _items_to_install.end(); iter++) {
	PoolItem_Ref r = *iter;

	Id id = (*iter)->satSolvable().id();
	if (id == ID_NULL) {
	    ERR << "Install: " << *iter << " not found" << endl;
	}
	MIL << "Install " << *iter << " with the SAT-Pool ID: " << id << endl;
	queue_push( &(_jobQueue), SOLVER_INSTALL_SOLVABLE );
        queue_push( &(_jobQueue), id );
    }

    for (PoolItemList::const_iterator iter = _items_to_remove.begin(); iter != _items_to_remove.end(); iter++) {
        sat::detail::IdType ident( (*iter)->satSolvable().ident().id() );
	MIL << "Delete " << *iter << " with the string ID: " << ident << endl;
	queue_push( &(_jobQueue), SOLVER_ERASE_SOLVABLE_NAME );
	queue_push( &(_jobQueue), ident);
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

    for (PoolItemList::const_iterator iter = _items_to_lock.begin(); iter != _items_to_lock.end(); iter++) {
        sat::detail::SolvableIdType ident( (*iter)->satSolvable().id() );
	if (iter->status().isInstalled()) {
	    MIL << "Lock installed item " << *iter << " with the string ID: " << ident << endl;
	    queue_push( &(_jobQueue), SOLVER_INSTALL_SOLVABLE );
	    queue_push( &(_jobQueue), ident );
	} else {
	    MIL << "Lock NOT installed item " << *iter << " with the string ID: " << ident << endl;
	    queue_push( &(_jobQueue), SOLVER_ERASE_SOLVABLE );
	    queue_push( &(_jobQueue), ident );
	}
    }

    _solv = solver_create( _SATPool, sat::Pool::instance().systemRepo().get() );
    sat::Pool::instance().setDirty();
    sat::Pool::instance().prepare();
    _solv->fixsystem = false;
    _solv->updatesystem = false;
    _solv->allowdowngrade = false;
    _solv->allowuninstall = false;
    _solv->noupdateprovide = false;

    // Solve !
    MIL << "Starting solving...." << endl;
    solver_solve( _solv, &(_jobQueue) );
    MIL << "....Solver end" << endl;

    // copying solution back to zypp pool
    //-----------------------------------------

    if (_solv->problems.count > 0 )
    {
	ERR << "Solverrun finished with an ERROR" << endl;
	return false;
    }

    /*  solvables to be installed */
    for (int i = 0; i < _solv->decisionq.count; i++)
    {
      Id p;
      p = _solv->decisionq.elements[i];
      if (p < 0 || !sat::Solvable(p))
	continue;
      if (sat::Solvable(p).repo().get() == _solv->installed)
	continue;

      PoolItem_Ref poolItem = _pool.find (sat::Solvable(p));
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

      PoolItem_Ref poolItem = _pool.find (sat::Solvable(i));
      if (poolItem) {
	  // Check if this is an update
	  CheckIfUpdate info;
	  invokeOnEach( _pool.byNameBegin( poolItem->name() ),
			_pool.byNameEnd( poolItem->name() ),
			functor::chain (resfilter::ByUninstalled (),			// ByUninstalled
					resfilter::ByKind( poolItem->kind() ) ),	// equal kind
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

    // clean up
    solver_free(_solv);
    _solv = NULL;
    queue_free( &(_jobQueue) );

    return true;
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

    bool operator()( PoolItem_Ref p)
    {
	problemSolution->addSingleAction (p, action);
	return true;
    }
};


std::string SATResolver::SATprobleminfoString(Id problem)
{
  string ret;
  Pool *pool = _solv->pool;
  Id probr;
  Id dep, source, target;
  Solvable *s, *s2;

  probr = solver_findproblemrule(_solv, problem);
  switch (solver_problemruleinfo(_solv, &(_jobQueue), probr, &dep, &source, &target))
  {
      case SOLVER_PROBLEM_UPDATE_RULE:
	  s = pool_id2solvable(pool, source);
	  ret = str::form (_("problem with installed package %s"), solvable2str(pool, s));
	  break;
      case SOLVER_PROBLEM_JOB_RULE:
	  ret = str::form (_("conflicting requests"));
	  break;
      case SOLVER_PROBLEM_JOB_NOTHING_PROVIDES_DEP:
	  ret = str::form (_("nothing provides requested %s"), dep2str(pool, dep));
	  break;
      case SOLVER_PROBLEM_NOT_INSTALLABLE:
	  s = pool_id2solvable(pool, source);
	  ret = str::form (_("%s is not installable"), solvable2str(pool, s));
	  break;
      case SOLVER_PROBLEM_NOTHING_PROVIDES_DEP:
	  s = pool_id2solvable(pool, source);
	  ret = str::form (_("nothing provides %s needed by %s"), dep2str(pool, dep), solvable2str(pool, s));
	  break;
      case SOLVER_PROBLEM_SAME_NAME:
	  s = pool_id2solvable(pool, source);
	  s2 = pool_id2solvable(pool, target);
	  ret = str::form (_("cannot install both %s and %s"), solvable2str(pool, s), solvable2str(pool, s2));
	  break;
      case SOLVER_PROBLEM_PACKAGE_CONFLICT:
	  s = pool_id2solvable(pool, source);
	  s2 = pool_id2solvable(pool, target);
	  ret = str::form (_("%s conflicts with %s provided by %s"), solvable2str(pool, s), dep2str(pool, dep), solvable2str(pool, s2));
	  break;
      case SOLVER_PROBLEM_PACKAGE_OBSOLETES:
	  s = pool_id2solvable(pool, source);
	  s2 = pool_id2solvable(pool, target);
	  ret = str::form (_("%s obsoletes %s provided by %s"), solvable2str(pool, s), dep2str(pool, dep), solvable2str(pool, s2));
	  break;
      case SOLVER_PROBLEM_DEP_PROVIDERS_NOT_INSTALLABLE:
	  s = pool_id2solvable(pool, source);
	  ret = str::form (_("%s requires %s, but none of the providers can be installed"), solvable2str(pool, s), dep2str(pool, dep));
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
	Solvable *s, *sd;

	MIL << "Encountered problems! Here are the solutions:\n" << endl;
	pcnt = 1;
	problem = 0;
	while ((problem = solver_next_problem(_solv, problem)) != 0) {
	    MIL << "Problem " <<  pcnt << ":" << endl;
	    MIL << "====================================" << endl;
	    string whatString = SATprobleminfoString(problem);
	    MIL << whatString << endl;
	    MIL << "------------------------------------" << endl;
	    ResolverProblem_Ptr resolverProblem = new ResolverProblem (whatString, "");
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
				s = pool->solvables + what;
				PoolItem_Ref poolItem = _pool.find (sat::Solvable(what));
				if (poolItem) {
				    if (_solv->installed && s->repo == _solv->installed) {
					problemSolution->addSingleAction (poolItem, REMOVE);
					string description = str::form (_("do not keep %s installed"),  solvable2str(pool, s) );
					MIL << description << endl;
					problemSolution->addDescription (description);
				    } else {
					problemSolution->addSingleAction (poolItem, REMOVE);
					string description = str::form (_("do not install %s"), solvable2str(pool, s));
					MIL << description << endl;
					problemSolution->addDescription (description);
				    }
				} else {
				    ERR << "SOLVER_INSTALL_SOLVABLE: No item found for " << id2str(pool, s->name) << "-"
					<<  id2str(pool, s->evr) << "." <<  id2str(pool, s->arch) << endl;
				}
			    }
				break;
			    case SOLVER_ERASE_SOLVABLE: {
				s = pool->solvables + what;
				PoolItem_Ref poolItem = _pool.find (sat::Solvable(what));
				if (poolItem) {
				    if (_solv->installed && s->repo == _solv->installed) {
					problemSolution->addSingleAction (poolItem, KEEP);
					string description = str::form (_("keep %s"), solvable2str(pool, s));
					MIL << description << endl;
					problemSolution->addDescription (description);
				    } else {
					problemSolution->addSingleAction (poolItem, INSTALL);
					string description = str::form (_("do not forbid installation of %s"), solvable2str(pool, s));
					MIL << description << endl;
					problemSolution->addDescription (description);
				    }
				} else {
				    ERR << "SOLVER_ERASE_SOLVABLE: No item found for " << id2str(pool, s->name) << "-" <<  id2str(pool, s->evr) << "." <<
					id2str(pool, s->arch) << endl;
				}
			    }
				break;
			    case SOLVER_INSTALL_SOLVABLE_NAME:
				{
				FindPackage info (problemSolution, KEEP);
				string package_name (id2str(pool, what));
				invokeOnEach( _pool.byNameBegin( package_name ),
					      _pool.byNameEnd( package_name ),
					      resfilter::ByUninstalled (),
					      functor::functorRef<bool,PoolItem> (info) );
				string description = str::form (_("do not install %s"), id2str(pool, what));
				MIL << description << endl;
				problemSolution->addDescription (description);
				}
				break;
			    case SOLVER_ERASE_SOLVABLE_NAME:
				{
				FindPackage info (problemSolution, KEEP);
				string package_name (id2str(pool, what));
				invokeOnEach( _pool.byNameBegin( package_name ),
					      _pool.byNameEnd( package_name ),
					      functor::chain (resfilter::ByInstalled (),			// ByInstalled
							      resfilter::ByTransact ()),			// will be deinstalled
					      functor::functorRef<bool,PoolItem> (info) );
				string description = str::form (_("keep %s"), id2str(pool, what));
				MIL << description << endl;
				problemSolution->addDescription (description);
				}
				break;
			    case SOLVER_INSTALL_SOLVABLE_PROVIDES:
				{
				Id p, *pp;
				FOR_PROVIDES(p, pp, what);
				{
				    PoolItem_Ref poolItem = _pool.find (sat::Solvable(p));
				    if (poolItem.status().isToBeInstalled()
					|| poolItem.status().staysUninstalled())
					problemSolution->addSingleAction (poolItem, KEEP);
				}
				string description = str::form (_("do not install a solvable providing %s"), dep2str(pool, what));
				MIL << description << endl;
				problemSolution->addDescription (description);
				}
				break;
			    case SOLVER_ERASE_SOLVABLE_PROVIDES:
				{
				Id p, *pp;
				FOR_PROVIDES(p, pp, what);
				{
				    PoolItem_Ref poolItem = _pool.find (sat::Solvable(p));
				    if (poolItem.status().isToBeUninstalled()
					|| poolItem.status().staysInstalled())
					problemSolution->addSingleAction (poolItem, KEEP);
				}
				string description = str::form (_("keep all solvables providing %s"), dep2str(pool, what));
				MIL << description << endl;
				problemSolution->addDescription (description);
				}
				break;
			    case SOLVER_INSTALL_SOLVABLE_UPDATE:
				{
				PoolItem_Ref poolItem = _pool.find (sat::Solvable(what));
				s = pool->solvables + what;
				if (poolItem) {
				    if (_solv->installed && s->repo == _solv->installed) {
					problemSolution->addSingleAction (poolItem, KEEP);
					string description = str::form (_("do not install most recent version of %s"), solvable2str(pool, s));
					MIL << description << endl;
					problemSolution->addDescription (description);
				    } else {
					ERR << "SOLVER_INSTALL_SOLVABLE_UPDATE " << poolItem << " is not selected for installation" << endl;
				    }
				} else {
				    ERR << "SOLVER_INSTALL_SOLVABLE_UPDATE: No item found for " << id2str(pool, s->name) << "-" <<  id2str(pool, s->evr) << "." <<
					id2str(pool, s->arch) << endl;
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
			s = pool->solvables + p;
			sd = rp ? pool->solvables + rp : 0;

			PoolItem_Ref itemFrom = _pool.find (sat::Solvable(p));
			if (rp)
			{
			    int gotone = 0;

			    PoolItem_Ref itemTo = _pool.find (sat::Solvable(rp));
			    if (itemFrom && itemTo) {
				problemSolution->addSingleAction (itemTo, INSTALL);
				problemSolution->addSingleAction (itemFrom, REMOVE);

				if (evrcmp(pool, s->evr, sd->evr, EVRCMP_COMPARE ) > 0)
				{
				    string description = str::form (_("downgrade of %s to %s"), solvable2str(pool, s), solvable2str(pool, sd));
				    MIL << description << endl;
				    problemSolution->addDescription (description);
				    gotone = 1;
				}
				if (!_solv->allowarchchange && s->name == sd->name && s->arch != sd->arch && policy_illegal_archchange(pool, s, sd))
				{
				    string description = str::form (_("architecture change of %s to %s"), solvable2str(pool, s), solvable2str(pool, sd));
				    MIL << description << endl;
				    problemSolution->addDescription (description);
				    gotone = 1;
				}
				if (!_solv->allowvendorchange && s->name == sd->name && s->vendor != sd->vendor && policy_illegal_vendorchange(pool, s, sd))
				{
				    string description = str::form (_("vendor change of [%s]%s to [%s]%s") , id2str(pool, s->vendor) , solvable2str(pool, s),
								      string(sd->vendor ?  id2str(pool, sd->vendor) : " (no vendor) ").c_str(),  solvable2str(pool, sd));
				    MIL << description << endl;
				    problemSolution->addDescription (description);
				    gotone = 1;
				}
				if (!gotone) {
				    string description = str::form (_("replacement of %s with %s"), solvable2str(pool, s), solvable2str(pool, sd));
				    MIL << description << endl;
				    problemSolution->addDescription (description);
				}
			    } else {
				ERR << id2str(pool, s->name) << "-" <<  id2str(pool, s->evr) << "." <<  id2str(pool, s->arch)
				    << " or "  << id2str(pool, sd->name) << "-" <<  id2str(pool, sd->evr) << "." <<  id2str(pool, sd->arch) << " not found" << endl;
			    }
			}
			else
			{
			    if (itemFrom) {
				string description = str::form (_("deinstallation of %s"), solvable2str(pool, s));
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



///////////////////////////////////////////////////////////////////
};// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

