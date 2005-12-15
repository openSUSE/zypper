/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _QueueItemConflict_h
#define _QueueItemConflict_h

#include <iosfwd>
#include <list>
#include <string.h>

#include <zypp/solver/detail/QueueItemConflictPtr.h>
#include <zypp/solver/detail/ResItem.h>
#include <zypp/solver/detail/Dependency.h>
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
      //	CLASS NAME : QueueItemConflict
      
      class QueueItemConflict : public QueueItem {
          REP_BODY(QueueItemConflict);
      
        private:
          constDependencyPtr _dep;
          constResItemPtr _conflicting_resItem;
      
          bool _actually_an_obsolete;
      
        public:
      
          QueueItemConflict (WorldPtr world, constDependencyPtr dep, constResItemPtr resItem);
          virtual ~QueueItemConflict();
      
          // ---------------------------------- I/O
      
          static std::string toString (const QueueItemConflict & item);
      
          virtual std::ostream & dumpOn(std::ostream & str ) const;
      
          friend std::ostream& operator<<(std::ostream&, const QueueItemConflict & item);
      
          std::string asString (void ) const;
      
          // ---------------------------------- accessors
      
          constDependencyPtr dependency (void) const { return _dep; }
          bool actuallyAnObsolete (void) const { return _actually_an_obsolete; }
          void setActuallyAnObsolete (void) { _actually_an_obsolete = true; }
      
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
#endif // _QueueItemConflict_h
