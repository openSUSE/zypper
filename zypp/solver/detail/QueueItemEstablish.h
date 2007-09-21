/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* QueueItemEstablish.h
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

#ifndef ZYPP_SOLVER_DETAIL_QUEUEITEMESTABLISH_H
#define ZYPP_SOLVER_DETAIL_QUEUEITEMESTABLISH_H

#include <iosfwd>
#include <list>
#include <string>

#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/QueueItem.h"
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
//	CLASS NAME : QueueItemEstablish

class QueueItemEstablish : public QueueItem {

  private:
    PoolItem_Ref _item;
    bool _soft;
    int _channel_priority;
    int _other_penalty;

    bool _explicitly_requested;

  public:

    QueueItemEstablish (const ResPool & pool, PoolItem_Ref item, bool soft = false);
    virtual ~QueueItemEstablish();

    // ---------------------------------- I/O

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<(std::ostream & str, const QueueItemEstablish & obj)
    { return obj.dumpOn (str); }

    // ---------------------------------- accessors

    PoolItem_Ref item (void) const { return _item; }

    int channelPriority (void) const { return _channel_priority; }
    void setChannelPriority (int channel_priority) { _channel_priority = channel_priority; }

    int otherPenalty (void) { return _other_penalty; }
    void setOtherPenalty (int other_penalty) { _other_penalty = other_penalty; }

    void setExplicitlyRequested (void) { _explicitly_requested = true; }

    // ---------------------------------- methods

    virtual bool process (const QueueItemList & mainQueue, ResolverContext_Ptr context, QueueItemList & qil);
    virtual QueueItem_Ptr copy (void) const;
    virtual int cmp (QueueItem_constPtr item) const;

    virtual bool isRedundant (ResolverContext_Ptr context) const { return false; }
    virtual bool isSatisfied (ResolverContext_Ptr context) const;
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

#endif // ZYPP_SOLVER_DETAIL_QUEUEITEMESTABLISH_H
