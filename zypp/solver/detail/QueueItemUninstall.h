/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* QueueItemUninstall.h
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

#ifndef _QueueItemUninstall_h
#define _QueueItemUninstall_h

#include <iosfwd>
#include <list>
#include <string.h>

#include <zypp/solver/detail/QueueItemUninstallPtr.h>
#include <zypp/solver/detail/ResItem.h>
#include <zypp/Capability.h>
#include <zypp/solver/detail/Channel.h>

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
      //	CLASS NAME : QueueItemUninstall
      
      class QueueItemUninstall : public QueueItem {
          REP_BODY(QueueItemUninstall);
      
        private:
          constResItemPtr _resItem;
          const std::string _reason;
          Capability _dep_leading_to_uninstall;
          constResItemPtr _upgraded_to;
      
          bool _explicitly_requested;
          bool _remove_only;
          bool _due_to_conflict;
          bool _due_to_obsolete;
          bool _unlink;
      
        public:
      
          QueueItemUninstall (WorldPtr world, constResItemPtr resItem, const std::string & reason);
          virtual ~QueueItemUninstall();
      
          // ---------------------------------- I/O
      
          static std::string toString (const QueueItemUninstall & item);
      
          virtual std::ostream & dumpOn(std::ostream & str ) const;
      
          friend std::ostream& operator<<(std::ostream&, const QueueItemUninstall & item);
      
          std::string asString (void ) const;
      
          // ---------------------------------- accessors
      
          void setDependency (const Capability & dep) { _dep_leading_to_uninstall = dep; }
          void setExplicitlyRequested (void) { _explicitly_requested = true; }
          void setRemoveOnly (void) { _remove_only = true; }
          void setUpgradedTo (constResItemPtr resItem) { _upgraded_to = resItem; }
          void setDueToConflict (void) { _due_to_conflict = true; }
          void setDueToObsolete (void) { _due_to_obsolete = true; }
          void setUnlink (void);
      
          // ---------------------------------- methods
      
          virtual bool process (ResolverContextPtr context, QueueItemList & qil);
          virtual QueueItemPtr copy (void) const;
          virtual int cmp (constQueueItemPtr item) const;
          virtual bool isRedundant (ResolverContextPtr context) const { return false; }
          virtual bool isSatisfied (ResolverContextPtr context) const { return false; }
      
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

#endif // _QueueItemUninstall_h
