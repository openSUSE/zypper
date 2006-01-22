/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* QueueItem.h
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

#ifndef ZYPP_SOLVER_DETAIL_QUEUEITEM_H
#define ZYPP_SOLVER_DETAIL_QUEUEITEM_H

#include <iosfwd>
#include <list>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/solver/detail/QueueItemPtr.h"
#include "zypp/solver/detail/ResolverContextPtr.h"
#include "zypp/solver/detail/ResolverInfo.h"
#include "zypp/solver/temporary/ResItem.h"
#include "zypp/solver/temporary/Channel.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

      typedef enum {
          QUEUE_ITEM_TYPE_UNKNOWN = 0,			// ordering is important !
          QUEUE_ITEM_TYPE_INSTALL,
          QUEUE_ITEM_TYPE_REQUIRE,
          QUEUE_ITEM_TYPE_BRANCH,
          QUEUE_ITEM_TYPE_GROUP,
          QUEUE_ITEM_TYPE_CONFLICT,
          QUEUE_ITEM_TYPE_UNINSTALL,
          QUEUE_ITEM_TYPE_LAST
      } QueueItemType;


      typedef std::list<QueueItem_Ptr> QueueItemList;

      #define CMP(a,b) (((a) < (b)) - ((b) < (a)))

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : QueueItem

      class QueueItem : public base::ReferenceCounted, private base::NonCopyable {
          

        private:

          QueueItemType _type;
          World_Ptr _world;

          int _priority;
          size_t _size;
          ResolverInfoList _pending_info;

        protected:

          QueueItem (QueueItemType type, World_Ptr world);

        public:

          virtual ~QueueItem();

          // ---------------------------------- I/O

          static std::string toString (const QueueItem & item);

          static std::string toString (const QueueItemList & itemlist, const std::string & sep = ", ");

          virtual std::ostream & dumpOn(std::ostream & str ) const;

          friend std::ostream& operator<<(std::ostream&, const QueueItem & item);

          virtual std::string asString (void ) const;

          // ---------------------------------- accessors

          World_Ptr world (void) const { return _world; }
          int priority (void) const { return _priority; }
          void setPriority (int priority) { _priority = priority; }
          int size (void) const { return _size; }

          // ---------------------------------- methods

          void copy (const QueueItem *from);

          bool isBranch (void) const { return _type == QUEUE_ITEM_TYPE_BRANCH; }
          bool isConflict (void) const { return _type == QUEUE_ITEM_TYPE_CONFLICT; }
          bool isGroup (void) const { return _type == QUEUE_ITEM_TYPE_GROUP; }
          bool isInstall (void) const { return _type == QUEUE_ITEM_TYPE_INSTALL; }
          bool isRequire (void) const { return _type == QUEUE_ITEM_TYPE_REQUIRE; }
          bool isUninstall (void) const { return _type == QUEUE_ITEM_TYPE_UNINSTALL; }

          virtual bool process (ResolverContext_Ptr context, QueueItemList & qil) = 0;
          virtual QueueItem_Ptr copy (void) const = 0;
          virtual int cmp (QueueItem_constPtr item) const = 0;
          int compare (QueueItem_constPtr item) const { return CMP(_type, item->_type); }

          //   isRedundant true == can be dropped from any branch
          virtual bool isRedundant (ResolverContext_Ptr context) const = 0;

          //   isSatisfied true == can be dropped from any queue, and any
          //   branch containing it can also be dropped
          virtual bool isSatisfied (ResolverContext_Ptr context) const = 0;

          void addInfo (ResolverInfo_Ptr);
          void logInfo (ResolverContext_Ptr);
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

#endif // ZYPP_SOLVER_DETAIL_QUEUEITEM_H
