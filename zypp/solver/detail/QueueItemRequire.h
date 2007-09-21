/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* QueueItemRequire.h
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

#ifndef ZYPP_SOLVER_DETAIL_QUEUEITEMREQUIRE_H
#define ZYPP_SOLVER_DETAIL_QUEUEITEMREQUIRE_H

#include <iosfwd>
#include <list>
#include <string>

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
//	CLASS NAME : QueueItemRequire_Ptr
//	CLASS NAME : QueueItemRequire_constPtr
///////////////////////////////////////////////////////////////////
DEFINE_PTR_TYPE(QueueItemRequire);

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : QueueItemRequire

class QueueItemRequire : public QueueItem {

  private:
    const Capability _capability;		// the required capability
    bool _soft;

    PoolItem_Ref _requiring_item;		// who's requiring it

    PoolItem_Ref _upgraded_item;
    PoolItem_Ref _lost_item;

    bool _remove_only;

  public:

    QueueItemRequire (const ResPool & pool, const Capability & cap, bool soft = false);
    virtual ~QueueItemRequire();

    // ---------------------------------- I/O

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream & operator<<(std::ostream & str, const QueueItemRequire & obj)
    { return obj.dumpOn (str); }

    // ---------------------------------- accessors

    bool isSoft (void) const { return _soft; }

    const Capability & capability (void) const { return _capability; }

    void setRemoveOnly (void) { _remove_only = true; }
    void setUpgradedPoolItem (PoolItem_Ref upgraded_item) { _upgraded_item = upgraded_item; }
    void setLostPoolItem (PoolItem_Ref lost_item) { _lost_item = lost_item; }

    // ---------------------------------- methods

    virtual bool process (const QueueItemList & mainQueue, ResolverContext_Ptr context, QueueItemList & qil);
    virtual QueueItem_Ptr copy (void) const;
    virtual int cmp (QueueItem_constPtr item) const;
    virtual bool isRedundant (ResolverContext_Ptr context) const { return false; }
    virtual bool isSatisfied (ResolverContext_Ptr context) const { return false; }

    void addPoolItem (PoolItem_Ref item);


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

#endif // ZYPP_SOLVER_DETAIL_QUEUEITEMREQUIRE_H
