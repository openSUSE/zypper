/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* QueueItemConflict.h
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

#ifndef ZYPP_SOLVER_DETAIL_QUEUEITEMCONFLICT_H
#define ZYPP_SOLVER_DETAIL_QUEUEITEMCONFLICT_H

#include <iosfwd>
#include <list>
#include <string>

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

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : QueueItemConflict

class QueueItemConflict : public QueueItem {


  private:
    const Capability _capability;		// the conflicting capability
    PoolItem_Ref _conflicting_item;		// the item which issued the conflict, can be 'empty' for 'extraConflicts'
    bool _soft;

    bool _actually_an_obsolete;
    bool _explicitly_requested;    

  public:

    QueueItemConflict (const ResPool & pool, const Capability & capability, PoolItem_Ref item, bool soft = false);
    virtual ~QueueItemConflict();

    // ---------------------------------- I/O

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<(std::ostream & str, const QueueItemConflict & obj)
    { return obj.dumpOn (str); }

    // ---------------------------------- accessors

    const Capability & capability (void) const { return _capability; }
    bool isSoft (void) const { return _soft; }
    bool actuallyAnObsolete (void) const { return _actually_an_obsolete; }
    void setActuallyAnObsolete (void) { _actually_an_obsolete = true; }
    void setExplicitlyRequested (void) { _explicitly_requested = true; }    

    // ---------------------------------- methods

    virtual bool process (const QueueItemList & mainQueue, ResolverContext_Ptr context, QueueItemList & qil);
    virtual QueueItem_Ptr copy (void) const;
    virtual int cmp (QueueItem_constPtr item) const;
    virtual bool isRedundant (ResolverContext_Ptr context) const { return false; }
    virtual bool isSatisfied (ResolverContext_Ptr context) const { return false; }

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
#endif // ZYPP_SOLVER_DETAIL_QUEUEITEMCONFLICT_H
