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

#include "zypp/solver/detail/QueueItem.h"
#include "zypp/solver/detail/QueueItemConflictPtr.h"
#include "zypp/solver/temporary/ResItem.h"
#include "zypp/solver/temporary/Channel.h"
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
          const Capability  _dep;
          ResItem_constPtr _conflicting_resItem;

          bool _actually_an_obsolete;

        public:

          QueueItemConflict (World_Ptr world, const Capability & dep, ResItem_constPtr resItem);
          virtual ~QueueItemConflict();

          // ---------------------------------- I/O

          static std::string toString (const QueueItemConflict & item);

          virtual std::ostream & dumpOn(std::ostream & str ) const;

          friend std::ostream& operator<<(std::ostream&, const QueueItemConflict & item);

          std::string asString (void ) const;

          // ---------------------------------- accessors

          const Capability & dependency (void) const { return _dep; }
          bool actuallyAnObsolete (void) const { return _actually_an_obsolete; }
          void setActuallyAnObsolete (void) { _actually_an_obsolete = true; }

          // ---------------------------------- methods

          virtual bool process (ResolverContext_Ptr context, QueueItemList & qil);
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
