/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Resolver.h
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

#ifndef ZYPP_SOLVER_DETAIL_RESOLVER_H
#define ZYPP_SOLVER_DETAIL_RESOLVER_H

#include <iosfwd>
#include <list>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/solver/detail/ResolverPtr.h"
#include "zypp/solver/detail/ResolverQueue.h"
#include "zypp/solver/temporary/ResItem.h"
#include "zypp/solver/temporary/Channel.h"
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
//	CLASS NAME : Resolver

      class Resolver : public base::ReferenceCounted, private base::NonCopyable {
          

        private:
          Channel_constPtr _current_channel;

          World_Ptr _world;

          int _timeout_seconds;
          bool _verifying;

          QueueItemList _initial_items;
          CResItemList _resItems_to_install;
          CResItemList _resItems_to_remove;
          CResItemList _resItems_to_verify;

          CapSet _extra_deps;
          CapSet _extra_conflicts;

          ResolverQueueList _pending_queues;
          ResolverQueueList _pruned_queues;
          ResolverQueueList _complete_queues;
          ResolverQueueList _deferred_queues;
          ResolverQueueList _invalid_queues;

          int _valid_solution_count;

          ResolverContext_Ptr _best_context;
          bool _timed_out;

        public:

          Resolver (World_Ptr world = NULL);
          virtual ~Resolver();

          // ---------------------------------- I/O

          static std::string toString (const Resolver & resolver);

          virtual std::ostream & dumpOn(std::ostream & str ) const;

          friend std::ostream& operator<<(std::ostream&, const Resolver & resolver);

          std::string asString (void ) const;

          // ---------------------------------- accessors

          QueueItemList initialItems () const { return _initial_items; }

          ResolverQueueList pendingQueues () const { return _pending_queues; }
          ResolverQueueList prunedQueues () const { return _pruned_queues; }
          ResolverQueueList completeQueues () const { return _complete_queues; }
          ResolverQueueList deferredQueues () const { return _deferred_queues; }
          ResolverQueueList invalidQueues () const { return _invalid_queues; }

          ResolverContext_Ptr bestContext (void) const { return _best_context; }

          // ---------------------------------- methods

          void setTimeout (int seconds) { _timeout_seconds = seconds; }

          World_Ptr world (void) const;			// returns global world if _world == NULL
          void setWorld (World_Ptr world) { _world = world; }

          void setCurrentChannel (Channel_constPtr channel) { _current_channel = channel; }
          void addSubscribedChannel (Channel_constPtr channel);

          void addResItemToInstall (ResItem_constPtr resItem);
          void addResItemsToInstallFromList (CResItemList & rl);

          void addResItemToRemove (ResItem_constPtr resItem);
          void addResItemsToRemoveFromList (CResItemList & rl);

          void addResItemToVerify (ResItem_constPtr resItem);

          void addExtraDependency (const Capability & dependency);
          void addExtraConflict (const Capability & dependency);

          void verifySystem (void);
          void resolveDependencies (void);

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

#endif // ZYPP_SOLVER_DETAIL_RESOLVER_H
