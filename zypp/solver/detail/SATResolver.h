/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* SATResolver.h
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

#ifndef ZYPP_SOLVER_DETAIL_SAT_RESOLVER_H
#define ZYPP_SOLVER_DETAIL_SAT_RESOLVER_H

extern "C"
{
#include <solv/solver.h>
#include <solv/pool.h>
}

#include <iosfwd>
#include <list>
#include <map>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/ResPool.h"
#include "zypp/base/SerialNumber.h"
#include "zypp/ProblemTypes.h"
#include "zypp/ResolverProblem.h"
#include "zypp/ProblemSolution.h"
#include "zypp/Capability.h"
#include "zypp/solver/detail/SolverQueueItem.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////

  namespace sat
  {
    class Transaction;
  }

  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : SATResolver
/**
 * \todo The way solver options are passed as individual booleans from Resolver
 * via solver::detail::Resolver to SATResolver is pedestrian and error prone.
 * Introdce a dedicated solver option structure which is passed down as a whole.
*/
class SATResolver : public base::ReferenceCounted, private base::NonCopyable {

  private:
    ResPool _pool;
    Pool *_SATPool;
    Solver *_solv;
    Queue _jobQueue;

    // list of problematic items (orphaned)
    PoolItemList _problem_items;

    // list populated by calls to addPoolItemTo*()
    PoolItemList _items_to_install;
    PoolItemList _items_to_remove;
    PoolItemList _items_to_lock;
    PoolItemList _items_to_keep;

    // solve results
    PoolItemList _result_items_to_install;
    PoolItemList _result_items_to_remove;

    bool _fixsystem;			// repair errors in rpm dependency graph
    bool _allowdowngrade;		// allow to downgrade installed solvable
    bool _allowarchchange;		// allow to change architecture of installed solvables
    bool _allowvendorchange;		// allow to change vendor of installed solvables
    bool _allowuninstall;		// allow removal of installed solvables
    bool _updatesystem;			// update
    bool _noupdateprovide;		// true: update packages needs not to provide old package
    bool _dosplitprovides;		// true: consider legacy split provides
    bool _onlyRequires;	                // true: consider required packages only
    bool _ignorealreadyrecommended;	// true: ignore recommended packages that were already recommended by the installed packages
    bool _distupgrade;
    bool _distupgrade_removeunsupported;
    bool _solveSrcPackages;		// false: generate no job rule for source packages selected in the pool
    bool _cleandepsOnRemove;		// whether removing a package should also remove no longer needed requirements

    // ---------------------------------- methods
    std::string SATprobleminfoString (Id problem, std::string &detail, Id &ignoreId);
    void resetItemTransaction (PoolItem item);

    // Create a SAT solver and reset solver selection in the pool (Collecting
    void solverInit(const PoolItemList & weakItems);
    // common solver run with the _jobQueue; Save results back to pool
    bool solving(const CapabilitySet & requires_caps = CapabilitySet(),
		 const CapabilitySet & conflict_caps = CapabilitySet());
    // cleanup solver
    void solverEnd();
    // set locks for the solver
    void setLocks();
    // set requirements for a running system
    void setSystemRequirements();

   // Checking if this solvable/item has a buddy which reflect the real
   // user visible description of an item
   // e.g. The release package has a buddy to the concerning product item.
   // This user want's the message "Product foo conflicts with product bar" and
   // NOT "package release-foo conflicts with package release-bar"
   // So these functions return the concerning buddy (e.g. product item)
    sat::Solvable mapSolvable (const Id &id);
    PoolItem mapItem (const PoolItem &item);

  public:

    SATResolver (const ResPool & pool, Pool *SATPool);
    virtual ~SATResolver();

    // ---------------------------------- I/O

    virtual std::ostream & dumpOn( std::ostream & str ) const;
    friend std::ostream& operator<<(std::ostream& str, const SATResolver & obj)
    { return obj.dumpOn (str); }

    ResPool pool (void) const;
    void setPool (const ResPool & pool) { _pool = pool; }

    // solver run with pool selected items
    bool resolvePool(const CapabilitySet & requires_caps,
		     const CapabilitySet & conflict_caps,
		     const PoolItemList & weakItems,
		     const std::set<Repository> & upgradeRepos
		     );
    // solver run with the given request queue
    bool resolveQueue(const SolverQueueItemList &requestQueue,
		      const PoolItemList & weakItems
		      );
    // searching for new packages
    void doUpdate();

    ResolverProblemList problems ();
    void applySolutions (const ProblemSolutionList &solutions);

    void addPoolItemToInstall (PoolItem item);
    void addPoolItemsToInstallFromList (PoolItemList & rl);

    void addPoolItemToLock (PoolItem item);
    void addPoolItemToKeep (PoolItem item);

    void addPoolItemToRemove (PoolItem item);
    void addPoolItemsToRemoveFromList (PoolItemList & rl);

    bool fixsystem () const {return _fixsystem;}
    void setFixsystem ( const bool fixsystem) { _fixsystem = fixsystem;}

    bool ignorealreadyrecommended () const {return _ignorealreadyrecommended;}
    void setIgnorealreadyrecommended ( const bool ignorealreadyrecommended) { _ignorealreadyrecommended = ignorealreadyrecommended;}

    bool distupgrade () const {return _distupgrade;}
    void setDistupgrade ( const bool distupgrade) { _distupgrade = distupgrade;}

    bool distupgrade_removeunsupported () const {return _distupgrade_removeunsupported;}
    void setDistupgrade_removeunsupported ( const bool distupgrade_removeunsupported) { _distupgrade_removeunsupported = distupgrade_removeunsupported;}

    bool allowdowngrade () const {return _allowdowngrade;}
    void setAllowdowngrade ( const bool allowdowngrade) { _allowdowngrade = allowdowngrade;}

    bool allowarchchange () const {return _allowarchchange;}
    void setAllowarchchange ( const bool allowarchchange) { _allowarchchange = allowarchchange;}

    bool allowvendorchange () const {return _allowvendorchange;}
    void setAllowvendorchange ( const bool allowvendorchange) { _allowvendorchange = allowvendorchange;}

    bool allowuninstall () const {return _allowuninstall;}
    void setAllowuninstall ( const bool allowuninstall) { _allowuninstall = allowuninstall;}

    bool updatesystem () const {return _updatesystem;}
    void setUpdatesystem ( const bool updatesystem) { _updatesystem = updatesystem;}

    bool noupdateprovide () const {return _noupdateprovide;}
    void setNoupdateprovide ( const bool noupdateprovide) { _noupdateprovide = noupdateprovide;}

    bool dosplitprovides () const {return _dosplitprovides;}
    void setDosplitprovides ( const bool dosplitprovides) { _dosplitprovides = dosplitprovides;}

    bool onlyRequires () const {return _onlyRequires;}
    void setOnlyRequires ( const bool onlyRequires) { _onlyRequires = onlyRequires;}

    bool solveSrcPackages() const 		{ return _solveSrcPackages; }
    void setSolveSrcPackages( bool state_r )	{ _solveSrcPackages = state_r; }

    bool cleandepsOnRemove() const 		{ return _cleandepsOnRemove; }
    void setCleandepsOnRemove( bool state_r )	{ _cleandepsOnRemove = state_r; }

    PoolItemList problematicUpdateItems( void ) const { return _problem_items; }

    PoolItemList resultItemsToInstall () { return _result_items_to_install; }
    PoolItemList resultItemsToRemove () { return _result_items_to_remove; }
    PoolItemList problematicUpdateItems() { return _problem_items; }

    sat::StringQueue autoInstalled() const;
    sat::StringQueue userInstalled() const;
};

///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

#endif // ZYPP_SOLVER_DETAIL_SAT_RESOLVER_H
