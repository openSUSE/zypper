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

#ifndef ZYPP_SOLVER_DETAIL_QUEUEITEM_H
#define ZYPP_SOLVER_DETAIL_QUEUEITEM_H
#ifndef ZYPP_USE_RESOLVER_INTERNALS
#error Do not directly include this file!
#else

#include <iosfwd>
#include <list>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/ResPool.h"

extern "C" {
  struct _Queue;
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

DEFINE_PTR_TYPE(SolverQueueItem);

DEFINE_PTR_TYPE(SolverQueueItemUpdate);
DEFINE_PTR_TYPE(SolverQueueItemDelete);
DEFINE_PTR_TYPE(SolverQueueItemInstall);
DEFINE_PTR_TYPE(SolverQueueItemInstallOneOf);
DEFINE_PTR_TYPE(SolverQueueItemLock);


typedef enum {
    QUEUE_ITEM_TYPE_UNKNOWN = 0,
    QUEUE_ITEM_TYPE_UPDATE,
    QUEUE_ITEM_TYPE_INSTALL,
    QUEUE_ITEM_TYPE_DELETE,
    QUEUE_ITEM_TYPE_INSTALL_ONE_OF,
    QUEUE_ITEM_TYPE_LOCK
} SolverQueueItemType;


typedef std::list<SolverQueueItem_Ptr> SolverQueueItemList;

#define CMP(a,b) (((a) < (b)) - ((b) < (a)))

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : SolverQueueItem

class SolverQueueItem : public base::ReferenceCounted, private base::NonCopyable {

  private:

    SolverQueueItemType _type;
    ResPool _pool;

  protected:

    SolverQueueItem (SolverQueueItemType type, const ResPool & pool);

  public:

    virtual ~SolverQueueItem();

    // ---------------------------------- I/O

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<(std::ostream & str, const SolverQueueItem & obj)
    { return obj.dumpOn (str); }
    friend std::ostream& operator<<(std::ostream & str, const SolverQueueItemList & itemlist);

    // ---------------------------------- accessors

    ResPool pool (void) const { return _pool; }

    // ---------------------------------- methods

    void copy (const SolverQueueItem *from);

    bool isDelete (void) const { return _type == QUEUE_ITEM_TYPE_DELETE; }
    bool isInstall (void) const { return _type == QUEUE_ITEM_TYPE_INSTALL; }
    bool isUpdate (void) const { return _type == QUEUE_ITEM_TYPE_UPDATE; }
    bool isLock (void) const { return _type == QUEUE_ITEM_TYPE_LOCK; }
    bool isInstallOneOf (void) const { return _type == QUEUE_ITEM_TYPE_INSTALL_ONE_OF; }


    virtual SolverQueueItem_Ptr copy (void) const = 0;
    virtual bool addRule (_Queue & q) =0 ;
    virtual int cmp (SolverQueueItem_constPtr item) const = 0;
    int compare (SolverQueueItem_constPtr item) const { return CMP(_type, item->_type); }

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
#endif // ZYPP_USE_RESOLVER_INTERNALS
#endif // ZYPP_SOLVER_DETAIL_QUEUEITEM_H
