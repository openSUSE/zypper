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

#ifndef _QueueItemBranch_h
#define _QueueItemBranch_h

#include <iosfwd>
#include <list>
#include <string.h>

#include <zypp/solver/detail/QueueItemBranchPtr.h>
#include <zypp/solver/detail/ResItem.h>
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
      //	CLASS NAME : QueueItemBranch
      
      class QueueItemBranch : public QueueItem {
          REP_BODY(QueueItemBranch);
      
        private:
          std::string _label;
          QueueItemList _possible_items;
      
        public:
      
          QueueItemBranch (WorldPtr world);
          virtual ~QueueItemBranch();
      
          // ---------------------------------- I/O
      
          static std::string toString (const QueueItemBranch & item);
      
          virtual std::ostream & dumpOn(std::ostream & str ) const;
      
          friend std::ostream& operator<<(std::ostream&, const QueueItemBranch & item);
      
          std::string asString (void ) const;
      
          // ---------------------------------- accessors
      
          QueueItemList possibleItems (void) const { return _possible_items; }
      
          const std::string & label (void) const { return _label; }
          void setLabel (const std::string & label) { _label = label; }
      
          bool isEmpty (void) const { return _possible_items.empty(); }
      
          // ---------------------------------- methods
      
          virtual bool process (ResolverContextPtr context, QueueItemList & qil);
          virtual QueueItemPtr copy (void) const;
          virtual int cmp (constQueueItemPtr item) const;
          virtual bool isRedundant (ResolverContextPtr context) const { return false; }
          virtual bool isSatisfied (ResolverContextPtr context) const { return false; }
      
          void addItem (QueueItemPtr subitem);
          bool contains (QueueItemPtr possible_subbranch);
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
#endif // _QueueItemBranch_h
