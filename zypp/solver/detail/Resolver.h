/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Resolver.h
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

#ifndef ZYPP_SOLVER_DETAIL_RESOLVER_H
#define ZYPP_SOLVER_DETAIL_RESOLVER_H

#include <iosfwd>
#include <list>
#include <map>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/ResPool.h"

#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/ResolverQueue.h"
#include "zypp/solver/detail/ResolverContext.h"
#include "zypp/solver/detail/ContextPool.h"

#include "zypp/ProblemTypes.h"
#include "zypp/ResolverProblem.h"
#include "zypp/ProblemSolution.h"
#include "zypp/UpgradeStatistics.h"

#include "zypp/CapSet.h"


/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////
	
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ItemCapKind
    //
    /** */
    struct ItemCapKind
    {
	public:
	Capability cap; //Capability which has triggerd this selection
	Dep capKind; //Kind of that capability
	PoolItem_Ref item; //Item which has triggered this selection
	bool initialInstallation; //This item has triggered the installation
	                          //Not already fullfilled requierement only.

	    ItemCapKind( PoolItem i, Capability c, Dep k, bool initial)
		: cap( c )
		, capKind( k )
		, item( i )
		, initialInstallation( initial )
	    { }
    };
    typedef std::multimap<PoolItem_Ref,ItemCapKind> ItemCapKindMap;
    typedef std::list<ItemCapKind> ItemCapKindList;	

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Resolver

class Resolver : public base::ReferenceCounted, private base::NonCopyable {

  private:
    ResPool _pool;

    unsigned _timeout_seconds;
    unsigned _maxSolverPasses;
    bool _verifying;
    bool _testing;
    
    // In order reducing solver time we are reducing the branches
    // by skipping resolvables which have worse architecture,edition
    // than a resolvable which provides the same cababilities.
    // BUT if there is no valid solution we will regard the "other"
    // resolvables in a second solver run too.
    bool _tryAllPossibilities; // Try ALL alternatives

    // list populated by calls to addPoolItemTo*()
    QueueItemList _initial_items;
    PoolItemList _items_to_install;
    PoolItemList _items_to_establish;
    PoolItemList _items_to_remove;
    PoolItemList _items_to_verify;
    PoolItemList _items_to_lockUninstalled;    
  
    // pool of valid contexts which are "recycled" in order to fasten the solver
    ContextPool contextPool;

    // list of problematic items after doUpgrade()
    PoolItemList _update_items;

    // Additional information about the solverrun
    ItemCapKindMap _isInstalledBy;
    ItemCapKindMap _installs;
    
    CapSet _extra_caps;
    CapSet _extra_conflicts;

    //typedef std::multimap<PoolItem_Ref,Capability> IgnoreMap;

    // These conflict should be ignored of the concering item
    IgnoreMap _ignoreConflicts;
    // These requires should be ignored of the concering item    
    IgnoreMap _ignoreRequires;
    // These obsoletes should be ignored of the concering item    
    IgnoreMap _ignoreObsoletes;    
    // Ignore architecture of the item
    PoolItemList _ignoreArchitecture;
    // Ignore the status "installed" of the item
    PoolItemList _ignoreInstalledItem;
    // Ignore the architecture of the item
    PoolItemList _ignoreArchitectureItem;    
    

    ResolverQueueList _pending_queues;
    ResolverQueueList _pruned_queues;
    ResolverQueueList _complete_queues;
    ResolverQueueList _deferred_queues;
    ResolverQueueList _invalid_queues;

    int _valid_solution_count;

    ResolverContext_Ptr _best_context;
    // Context of the last establishing call ( without any transaction )
    ResolverContext_Ptr _establish_context;    
    bool _timed_out;

    std::set<Repository> _subscribed;

    Arch _architecture;

    bool _forceResolve; // remove items which are conflicts with others or
                        // have unfulfilled requirements.
                        // This behaviour is favourited by ZMD
    bool _upgradeMode;  // Resolver has been called with doUpgrade
    bool _preferHighestVersion; // Prefer the result with the newest version
                                //if there are more solver results. 

    // helpers
    bool doesObsoleteCapability (PoolItem_Ref candidate, const Capability & cap);
    bool doesObsoleteItem (PoolItem_Ref candidate, PoolItem_Ref installed);

    void collectResolverInfo (void);
    

  public:

    Resolver (const ResPool & pool);
    virtual ~Resolver();

    // ---------------------------------- I/O

    virtual std::ostream & dumpOn( std::ostream & str ) const;
    friend std::ostream& operator<<(std::ostream& str, const Resolver & obj)
    { return obj.dumpOn (str); }
    void dumpTaskList(const PoolItemList &install, const PoolItemList &remove );
    
    // ---------------------------------- accessors

    QueueItemList initialItems () const { return _initial_items; }

    ResolverQueueList pendingQueues () const { return _pending_queues; }
    ResolverQueueList prunedQueues () const { return _pruned_queues; }
    ResolverQueueList completeQueues () const { return _complete_queues; }
    ResolverQueueList deferredQueues () const { return _deferred_queues; }
    ResolverQueueList invalidQueues () const { return _invalid_queues; }

    ResolverContext_Ptr bestContext (void) const { return _best_context; }

    /** depending on the last solver result, either return bestContext()
        of the first invalid context */
    ResolverContext_Ptr context (void) const;

    // ---------------------------------- methods

    void setTimeout (int seconds) { _timeout_seconds = seconds; }
    void setMaxSolverPasses (int count) { _maxSolverPasses = count; }
    int timeout () { return _timeout_seconds; }
    int maxSolverPasses () { return _maxSolverPasses; }

    ResPool pool (void) const;
    void setPool (const ResPool & pool) { _pool = pool; }

    void addSubscribedSource (Repository source);

    void addPoolItemToInstall (PoolItem_Ref item);
    void addPoolItemsToInstallFromList (PoolItemList & rl);

    void addPoolItemToLockUninstalled (PoolItem_Ref item);

    void addPoolItemToRemove (PoolItem_Ref item);
    void addPoolItemsToRemoveFromList (PoolItemList & rl);

    void addPoolItemToEstablish (PoolItem_Ref item);
    void addPoolItemsToEstablishFromList (PoolItemList & rl);

    void addPoolItemToVerify (PoolItem_Ref item);

    void addExtraCapability (const Capability & capability);
    void addExtraConflict (const Capability & capability);

    void addIgnoreConflict (const PoolItem_Ref item,
			    const Capability & capability);
    void addIgnoreRequires (const PoolItem_Ref item,
			    const Capability & capability);
    void addIgnoreObsoletes (const PoolItem_Ref item,
			     const Capability & capability);
    void addIgnoreInstalledItem (const PoolItem_Ref item);
    void addIgnoreArchitectureItem (const PoolItem_Ref item);    

    void setForceResolve (const bool force) { _forceResolve = force; }
    const bool forceResolve() { return _forceResolve; }
    void setPreferHighestVersion (const bool highestVersion) { _preferHighestVersion = highestVersion; }
    const bool preferHighestVersion() { return _preferHighestVersion; }

    void setTryAllPossibilities (const bool tryAllPossibilities) { _tryAllPossibilities = tryAllPossibilities; }
    const bool tryAllPossibilities () { return _tryAllPossibilities; };
    
    bool verifySystem (bool considerNewHardware = false);
    void establishState (ResolverContext_Ptr context = NULL);
    bool establishPool (void);
    void freshenState( ResolverContext_Ptr context = NULL, bool resetAfterSolve = true );
    bool freshenPool( bool resetAfterSolve = true );
    bool resolveDependencies (const ResolverContext_Ptr context = NULL);
    bool resolvePool( bool tryAllPossibilities = false );

    bool transactResObject( ResObject::constPtr robj,
			    bool install = true,
			    bool recursive = false);
    bool transactResKind( Resolvable::Kind kind );
    void transactReset( ResStatus::TransactByValue causer );

    void doUpgrade( zypp::UpgradeStatistics & opt_stats_r );
    PoolItemList problematicUpdateItems( void ) const { return _update_items; }


    ResolverProblemList problems (const bool ignoreValidSolution = false) const;
    void applySolutions (const ProblemSolutionList &solutions);
    // returns a string list of ResolverInfo of the LAST not valid solution
    std::list<std::string> problemDescription( void ) const;

    // reset all SOLVER transaction in pool
    void undo(void);

    // Get more information about the solverrun
    // Which item will be installed by another item or triggers an item for
    // installation    
    const ItemCapKindList isInstalledBy (const PoolItem_Ref item);
    const ItemCapKindList installs (const PoolItem_Ref item);

    // only for testsuite
    void reset (const bool resetValidResults = false);

    Arch architecture() const { return _architecture; }
    void setArchitecture( const Arch & arch) { _architecture = arch; }

    bool testing(void) const { return _testing; }
    void setTesting( bool testing ) { _testing = testing; }
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

#endif // ZYPP_SOLVER_DETAIL_RESOLVER_H
