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
extern "C" {
#include "satsolver/solver.h"
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

    unsigned _timeout_seconds;
    unsigned _maxSolverPasses;
    bool _testing;
    int _valid_solution_count;
    bool _timed_out;
    Arch _architecture;

    // list populated by calls to addPoolItemTo*()
    PoolItemList _items_to_install;
    PoolItemList _items_to_remove;
    PoolItemList _items_to_lockUninstalled;
    PoolItemList _items_to_keep;    

  public:

    SATResolver (const ResPool & pool);
    virtual ~SATResolver();

    // ---------------------------------- I/O

    virtual std::ostream & dumpOn( std::ostream & str ) const;
    friend std::ostream& operator<<(std::ostream& str, const SATResolver & obj)
    { return obj.dumpOn (str); }


    // ---------------------------------- methods

    void setTimeout (int seconds) { _timeout_seconds = seconds; }
    void setMaxSolverPasses (int count) { _maxSolverPasses = count; }
    int timeout () const { return _timeout_seconds; }
    int maxSolverPasses () const { return _maxSolverPasses; }
    ResPool pool (void) const;
    void setPool (const ResPool & pool) { _pool = pool; }

    bool resolvePool();

//    ResolverProblemList problems () const;
//    void applySolutions (const ProblemSolutionList &solutions);

    Arch architecture() const { return _architecture; }
    void setArchitecture( const Arch & arch) { _architecture = arch; }

    bool testing(void) const { return _testing; }
    void setTesting( bool testing ) { _testing = testing; }

    void addPoolItemToInstall (PoolItem_Ref item);
    void addPoolItemsToInstallFromList (PoolItemList & rl);

    void addPoolItemToLockUninstalled (PoolItem_Ref item);
    void addPoolItemToKepp (PoolItem_Ref item);

    void addPoolItemToRemove (PoolItem_Ref item);
    void addPoolItemsToRemoveFromList (PoolItemList & rl);
    
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
