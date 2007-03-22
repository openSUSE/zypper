/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverQueue.h
 *
 * Copyright (C) 2007 SUSE Linux Products GmbH
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

#ifndef ZYPP_SOLVER_DETAIL_CONTEXTPOOL_H
#define ZYPP_SOLVER_DETAIL_CONTEXTPOOL_H

#include <iosfwd>
#include <list>
#include <map>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/QueueItem.h"

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

	
typedef std::list <ResolverContext_Ptr> ResolverContextList;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ContextPool
/**
 * Save the last resolver results in a list.
 * The next sovler run can search an already existing rusult with which
 * he can continue.<br>
 *
 * */

class ContextPool : public base::ReferenceCounted, private base::NonCopyable {

  private:
    /** maximum stored solver results */
    unsigned maxContext;
    /** List of sucessful solver runs */
    ResolverContextList contextList;

  public:
    ContextPool ();
    /** 
     * Constructor
     *
     * @param maxCount maximum stored solver results
     * */
    ContextPool (const int maxCount);
    virtual ~ContextPool();

    // ---------------------------------- I/O

    friend std::ostream& operator<<(std::ostream&, const ContextPool & contextPool);


    // ---------------------------------- methods
    /** 
     * Count of sucessful solver runs 
     *
     * */     
    int contextListSize() const { return contextList.size(); }
    /** 
     * Add an additional sucessful solver run.
     *
     * @param context Solver context
     * @param installItems List of items which are selected by the user
     * @param deleteItems List of items which are selected by the user
     * @param lockUninstalledItems List of items which are selected by the user
     *
     * */         
    void addContext (ResolverContext_Ptr context,
		     const PoolItemList & installItems,
		     const PoolItemList & deleteItems,
		     const PoolItemList & lockUninstalledItems);
    /** 
     * Find a solver result in order to use it for the next solver run.
     *
     * @param context Solver context
     * @param installItems List of items which are selected by the user
     * @param deleteItems List of items which are selected by the user
     * @param lockUninstalledItems List of items which are selected by the user
     * @return solver context
     * */             
    ResolverContext_Ptr findContext (PoolItemList & installItems,
				     PoolItemList & deleteItems,
				     const PoolItemList & lockUninstalledItems);

    /** 
     * Delete all sucessful solver run.
     *
     * */         
    void reset () { contextList.clear(); }
    
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

#endif // ZYPP_SOLVER_DETAIL_CONTEXTPOOL_H

