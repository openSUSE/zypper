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

#ifndef _QueueItemRequire_h
#define _QueueItemRequire_h

#include <iosfwd>
#include <list>
#include <string>

#include "zypp/solver/detail/QueueItem.h"
#include "zypp/solver/detail/QueueItemRequirePtr.h"
#include "zypp/solver/detail/ResItem.h"
#include "zypp/Capability.h"
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
      //	CLASS NAME : QueueItemRequire

      class QueueItemRequire : public QueueItem {


        private:
          const Capability _dep;
          ResItem_constPtr _requiring_resItem;
          ResItem_constPtr _upgraded_resItem;
          ResItem_constPtr _lost_resItem;
          bool _remove_only;
          bool _is_child;

        public:

          QueueItemRequire (World_Ptr world, const Capability & dep);
          virtual ~QueueItemRequire();

          // ---------------------------------- I/O

          static std::string toString (const QueueItemRequire & item);

          virtual std::ostream & dumpOn(std::ostream & str ) const;

          friend std::ostream& operator<<(std::ostream&, const QueueItemRequire & item);

          std::string asString (void ) const;

          // ---------------------------------- accessors

          const Capability & dependency (void) const { return _dep; }

          void setRemoveOnly (void) { _remove_only = true; }
          void setUpgradedResItem (ResItem_constPtr upgraded_resItem) { _upgraded_resItem = upgraded_resItem; }
          void setLostResItem (ResItem_constPtr lost_resItem) { _lost_resItem = lost_resItem; }

          // ---------------------------------- methods

          virtual bool process (ResolverContext_Ptr context, QueueItemList & qil);
          virtual QueueItem_Ptr copy (void) const;
          virtual int cmp (QueueItem_constPtr item) const;
          virtual bool isRedundant (ResolverContext_Ptr context) const { return false; }
          virtual bool isSatisfied (ResolverContext_Ptr context) const { return false; }

          void addResItem (ResItem_constPtr resItem);


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

#endif // _QueueItemRequire_h
