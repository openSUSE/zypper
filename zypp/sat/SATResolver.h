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
extern "C" {
#include "satsolver/solver.h"
#include "satsolver/pool.h"
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


///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : SATResolver

class SATResolver : public base::ReferenceCounted, private base::NonCopyable {

  private:
    ResPool _pool;
    Pool *_SATPool;
    Solver *_solv;
    Queue _jobQueue;

    // list populated by calls to addPoolItemTo*()
    PoolItemList _items_to_install;
    PoolItemList _items_to_remove;
    PoolItemList _items_to_lock;

    bool _fixsystem;			/* repair errors in rpm dependency graph */
    bool _allowdowngrade;		/* allow to downgrade installed solvable */
    bool _allowarchchange;		/* allow to change architecture of installed solvables */
    bool _allowvendorchange;		/* allow to change vendor of installed solvables */
    bool _allowuninstall;		/* allow removal of installed solvables */
    bool _updatesystem;			/* distupgrade */
    bool _allowvirtualconflicts;	/* false: conflicts on package name, true: conflicts on package provides */
    bool _noupdateprovide;		/* true: update packages needs not to provide old package */
    bool _dosplitprovides;		/* true: consider legacy split provides */
    
    // ---------------------------------- methods
    std::string SATprobleminfoString (Id problem);

  public:

    SATResolver (const ResPool & pool, Pool *SATPool);
    virtual ~SATResolver();

    // ---------------------------------- I/O

    virtual std::ostream & dumpOn( std::ostream & str ) const;
    friend std::ostream& operator<<(std::ostream& str, const SATResolver & obj)
    { return obj.dumpOn (str); }

    ResPool pool (void) const;
    void setPool (const ResPool & pool) { _pool = pool; }

    bool resolvePool(const CapabilitySet & requires_caps,
		     const CapabilitySet & conflict_caps);

    ResolverProblemList problems ();
    void applySolutions (const ProblemSolutionList &solutions);

    void addPoolItemToInstall (PoolItem item);
    void addPoolItemsToInstallFromList (PoolItemList & rl);

    void addPoolItemToLock (PoolItem item);

    void addPoolItemToRemove (PoolItem item);
    void addPoolItemsToRemoveFromList (PoolItemList & rl);

    bool fixsystem () const {return _fixsystem;}
    void setFixsystem ( const bool fixsystem) { _fixsystem = fixsystem;}

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
    
    bool allowvirtualconflicts () const {return _allowvirtualconflicts;}
    void setAllowvirtualconflicts ( const bool allowvirtualconflicts) { _allowvirtualconflicts = allowvirtualconflicts;}
    
    bool noupdateprovide () const {return _noupdateprovide;}
    void setNoupdateprovide ( const bool noupdateprovide) { _noupdateprovide = noupdateprovide;}
    
    bool dosplitprovides () const {return _dosplitprovides;}
    void setDosplitprovides ( const bool dosplitprovides) { _dosplitprovides = dosplitprovides;}

    PoolItemList whoProvides(Capability cap);
    bool doesObsoleteItem (PoolItem candidate, PoolItem installed);
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
