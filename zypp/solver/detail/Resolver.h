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
#include "zypp/TriBool.h"
#include "zypp/base/SerialNumber.h"

#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/SolverQueueItem.h"

#include "zypp/ProblemTypes.h"
#include "zypp/ResolverProblem.h"
#include "zypp/ProblemSolution.h"
#include "zypp/UpgradeStatistics.h"
#include "zypp/Capabilities.h"
#include "zypp/Capability.h"


/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

    class SATResolver;


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
	PoolItem item; //Item which has triggered this selection
	bool initialInstallation; //This item has triggered the installation
	                          //Not already fullfilled requierement only.

    ItemCapKind() : capKind("FRESHENS") {}
	    ItemCapKind( PoolItem i, Capability c, Dep k, bool initial)
		: cap( c )
		, capKind( k )
		, item( i )
		, initialInstallation( initial )
	    { }
    };
    typedef std::multimap<PoolItem,ItemCapKind> ItemCapKindMap;
    typedef std::list<ItemCapKind> ItemCapKindList;
	

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Resolver

class Resolver : public base::ReferenceCounted, private base::NonCopyable {

  private:
    ResPool _pool;
    SATResolver *_satResolver;
    SerialNumberWatcher _poolchanged;
    bool _testing;

    // list of problematic items after doUpgrade()
    PoolItemList _problem_items;

    // list of not supported packages
    PoolItemSet _unmaintained_items;    

    CapabilitySet _extra_requires;
    CapabilitySet _extra_conflicts;
    
    // Regard dependencies of the item weak onl
    PoolItemList _addWeak;

    bool _forceResolve;           // remove items which are conflicts with others or
                                  // have unfulfilled requirements.
                                  // This behaviour is favourited by ZMD
    bool _upgradeMode;            // Resolver has been called with doUpgrade
    bool _verifying;              // The system will be checked
    TriBool _onlyRequires; 	  // do install required resolvables only
                                  // no recommended resolvables, language
                                  // packages, hardware packages (modalias)

    bool _ignorealreadyrecommended;   //ignore recommended packages that were already recommended by the installed packages

    // Additional QueueItems which has to be regarded by the solver
    // This will be used e.g. by solution actions
    solver::detail::SolverQueueItemList _removed_queue_items;
    solver::detail::SolverQueueItemList _added_queue_items;

    // Additional information about the solverrun
    ItemCapKindMap _isInstalledBy;
    ItemCapKindMap _installs;
    ItemCapKindMap _satifiedByInstalled;
    ItemCapKindMap _installedSatisfied;
    
    // helpers
    bool doesObsoleteItem (PoolItem candidate, PoolItem installed);
    void collectResolverInfo (void);    

    // Unmaintained packages which does not fit to the updated system
    // (broken dependencies) will be deleted.
    // returns true if solving was successful
    bool checkUnmaintainedItems ();

    void solverInit();

  public:

    Resolver (const ResPool & pool);
    virtual ~Resolver();

    // ---------------------------------- I/O

    virtual std::ostream & dumpOn( std::ostream & str ) const;
    friend std::ostream& operator<<(std::ostream& str, const Resolver & obj)
    { return obj.dumpOn (str); }

    // ---------------------------------- methods

    ResPool pool (void) const;
    void setPool (const ResPool & pool) { _pool = pool; }

    void addExtraRequire (const Capability & capability);
    void removeExtraRequire (const Capability & capability);
    void addExtraConflict (const Capability & capability);
    void removeExtraConflict (const Capability & capability);

    void removeQueueItem (const SolverQueueItem_Ptr item);
    void addQueueItem (const SolverQueueItem_Ptr item);    

    const CapabilitySet extraRequires () { return _extra_requires; }
    const CapabilitySet extraConflicts () { return _extra_conflicts; }

    void addWeak (const PoolItem item);

    void setForceResolve (const bool force) { _forceResolve = force; }
    bool forceResolve() { return _forceResolve; }

    void setIgnorealreadyrecommended (const bool ignorealreadyrecommended)
	{ _ignorealreadyrecommended = ignorealreadyrecommended; }
    bool ignorealreadyrecommended() { return _ignorealreadyrecommended; }    

    void setOnlyRequires (const TriBool state)
	{ _onlyRequires = state; }
    TriBool onlyRequires () { return _onlyRequires; }

    bool verifySystem ();
    bool resolvePool();
    bool resolveQueue(solver::detail::SolverQueueItemList & queue);    
    void doUpdate();

    bool doUpgrade( zypp::UpgradeStatistics & opt_stats_r );
    PoolItemList problematicUpdateItems( void ) const { return _problem_items; }

    ResolverProblemList problems () const;
    void applySolutions (const ProblemSolutionList &solutions);

    // reset all SOLVER transaction in pool
    void undo(void);

    void reset (bool keepExtras = false );

    bool testing(void) const { return _testing; }
    void setTesting( bool testing ) { _testing = testing; }

    // Get more information about the solverrun
    // Which item will be installed by another item or triggers an item for
    // installation
    const ItemCapKindList isInstalledBy (const PoolItem item);
    const ItemCapKindList installs (const PoolItem item);
    const ItemCapKindList satifiedByInstalled (const PoolItem item);    
    const ItemCapKindList installedSatisfied (const PoolItem item);
    
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
