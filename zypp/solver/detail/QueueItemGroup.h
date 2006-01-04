/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* QueueItemGroup.h
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

#ifndef _QueueItemGroup_h
#define _QueueItemGroup_h

#include <iosfwd>
#include <list>
#include <string>

#include "zypp/solver/detail/QueueItem.h"
#include "zypp/solver/detail/QueueItemGroupPtr.h"
#include "zypp/solver/detail/ResItem.h"
#include "zypp/solver/detail/Channel.h"

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
      //	CLASS NAME : QueueItemGroup

      class QueueItemGroup : public QueueItem {


        private:
          QueueItemList _subitems;

        public:

          QueueItemGroup (World_Ptr world);
          virtual ~QueueItemGroup();

          // ---------------------------------- I/O

          static std::string toString (const QueueItemGroup & item);

          virtual std::ostream & dumpOn(std::ostream & str ) const;

          friend std::ostream& operator<<(std::ostream&, const QueueItemGroup & item);

          std::string asString (void ) const;

          // ---------------------------------- accessors


          // ---------------------------------- methods

          virtual bool process (ResolverContext_Ptr context, QueueItemList & qil);
          virtual QueueItem_Ptr copy (void) const;
          virtual int cmp (QueueItem_constPtr item) const;
          virtual bool isRedundant (ResolverContext_Ptr context) const { return false; }
          virtual bool isSatisfied (ResolverContext_Ptr context) const { return false; }

          void addItem (QueueItem_Ptr subitem);
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
#endif // _QueueItemGroup_h
