/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* QueueItem.h
 *
 * Copyright (C) 2008 SUSE Linux Products GmbH
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

#ifndef ZYPP_SOLVER_DETAIL_QUEUEITEMUPDATE_H
#define ZYPP_SOLVER_DETAIL_QUEUEITEMUPDATE_H

#include <iosfwd>
#include <string>

#include "zypp/solver/detail/SolverQueueItem.h"
#include "zypp/PoolItem.h"

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
//	CLASS NAME : SolverQueueItemUpdate

class SolverQueueItemUpdate : public SolverQueueItem {
    
  private:

    PoolItem _item;  	// the item to-be-updated
    bool _soft;         // if triggered by a soft requirement (a recommends)

  public:

    SolverQueueItemUpdate (const ResPool & pool, const PoolItem & item, bool soft = false);
    virtual ~SolverQueueItemUpdate();
    
    // ---------------------------------- I/O

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<(std::ostream & str, const SolverQueueItemUpdate & obj)
    { return obj.dumpOn (str); }

    // ---------------------------------- accessors

    bool isSoft (void) const { return _soft; }    

    // ---------------------------------- methods
    
    virtual bool addRule (sat::detail::CQueue & q);
    virtual SolverQueueItem_Ptr copy (void) const;
    virtual int cmp (SolverQueueItem_constPtr item) const;
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

#endif // ZYPP_SOLVER_DETAIL_QUEUEITEMUPDATE_H
