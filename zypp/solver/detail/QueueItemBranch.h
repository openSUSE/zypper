/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* QueueItemBranch.h
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

#ifndef ZYPP_SOLVER_DETAIL_QUEUEITEMBRANCH_H
#define ZYPP_SOLVER_DETAIL_QUEUEITEMBRANCH_H

#include <iosfwd>
#include <list>
#include <string>

#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/QueueItem.h"

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
//	CLASS NAME : QueueItemBranch

class QueueItemBranch : public QueueItem {


  private:
    std::string _label;
    QueueItemList _possible_qitems;

  public:

    QueueItemBranch (const ResPool & pool);
    virtual ~QueueItemBranch();

    // ---------------------------------- I/O

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<(std::ostream & str, const QueueItemBranch & obj)
    { return obj.dumpOn (str); }

    // ---------------------------------- accessors

    QueueItemList possibleQItems (void) const { return _possible_qitems; }

    const std::string & label (void) const { return _label; }
    void setLabel (const std::string & label) { _label = label; }

    bool isEmpty (void) const { return _possible_qitems.empty(); }

    // ---------------------------------- methods

    virtual bool process (const QueueItemList & mainQueue, ResolverContext_Ptr context, QueueItemList & qil);
    virtual QueueItem_Ptr copy (void) const;
    virtual int cmp (QueueItem_constPtr item) const;
    virtual bool isRedundant (ResolverContext_Ptr context) const { return false; }
    virtual bool isSatisfied (ResolverContext_Ptr context) const { return false; }

    void addItem (QueueItem_Ptr subitem);
    bool contains (QueueItem_Ptr possible_subbranch);
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
#endif // ZYPP_SOLVER_DETAIL_QUEUEITEMBRANCH_H
