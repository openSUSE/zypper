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
#include "zypp/base/SerialNumber.h"

#include "zypp/solver/detail/Types.h"

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
    PoolItemList _unmaintained_items;    

    CapabilitySet _extra_requires;
    CapabilitySet _extra_conflicts;
    
    typedef std::multimap<PoolItem,Capability> IgnoreMap;

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
    // Ignore the vendor of the item
    PoolItemList _ignoreVendorItem;


    bool _forceResolve; // remove items which are conflicts with others or
                        // have unfulfilled requirements.
                        // This behaviour is favourited by ZMD
    bool _upgradeMode;  // Resolver has been called with doUpgrade
    bool _verifying;    // The system will be checked

    // helpers
    bool doesObsoleteCapability (PoolItem candidate, const Capability & cap);
    bool doesObsoleteItem (PoolItem candidate, PoolItem installed);


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

    const CapabilitySet extraRequires () { return _extra_requires; }
    const CapabilitySet extraConflicts () { return _extra_conflicts; }

    void addIgnoreConflict (const PoolItem item,
			    const Capability & capability);
    void addIgnoreRequires (const PoolItem item,
			    const Capability & capability);
    void addIgnoreObsoletes (const PoolItem item,
			     const Capability & capability);
    void addIgnoreInstalledItem (const PoolItem item);
    void addIgnoreArchitectureItem (const PoolItem item);
    void addIgnoreVendorItem (const PoolItem item);    

    void setForceResolve (const bool force) { _forceResolve = force; }
    bool forceResolve() { return _forceResolve; }

    bool verifySystem ();
    bool resolvePool();

    void doUpgrade( zypp::UpgradeStatistics & opt_stats_r );
    PoolItemList problematicUpdateItems( void ) const { return _problem_items; }

    ResolverProblemList problems () const;
    void applySolutions (const ProblemSolutionList &solutions);

    // reset all SOLVER transaction in pool
    void undo(void);

    void reset (bool keepExtras = false );

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
