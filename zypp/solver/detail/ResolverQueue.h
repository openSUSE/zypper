/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ResolverQueue.h
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

#ifndef _ResolverQueue_h
#define _ResolverQueue_h

#include <iosfwd>
#include <list>
#include <map>
#include <string.h>

#include <zypp/solver/detail/ResolverQueuePtr.h>
#include <zypp/solver/detail/ResolverContextPtr.h>
#include <zypp/solver/detail/QueueItem.h>
#include <zypp/solver/detail/ResItemPtr.h>
#include <zypp/Capability.h>

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////
      
      typedef std::list <ResolverQueuePtr> ResolverQueueList;
      
      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : ResolverQueue
      
      class ResolverQueue : public CountedRep {
      
          REP_BODY(ResolverQueue);
      
        private:
      
          ResolverContextPtr _context;
          QueueItemList _items;
      
        public:
          ResolverQueue (ResolverContextPtr context = NULL);
          virtual ~ResolverQueue();
      
          // ---------------------------------- I/O
      
          static std::string toString (const ResolverQueue & context);
          virtual std::ostream & dumpOn(std::ostream & str ) const;
          friend std::ostream& operator<<(std::ostream&, const ResolverQueue & context);
          std::string asString (void ) const;
      
          // ---------------------------------- accessors
      
          ResolverContextPtr context (void) const { return _context; }
          QueueItemList items(void) const { return _items; }
      
          // ---------------------------------- methods
      
      
          void addResItemToInstall (constResItemPtr resItem);
          void addResItemToRemove (constResItemPtr resItem, bool remove_only_mode);
          void addResItemToVerify (constResItemPtr resItem);
          void addExtraDependency (const Capability & dep);
          void addExtraConflict (const Capability & dep);
          void addItem (QueueItemPtr item);
      
          bool isEmpty () const { return _items.empty(); }
          bool isInvalid ();
          bool containsOnlyBranches ();
      
          bool processOnce ();
          void process ();
      
          void splitFirstBranch (ResolverQueueList & new_queues, ResolverQueueList & deferred_queues);
      
          void spew ();
      
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

#endif // _ResolverQueue_h
 
