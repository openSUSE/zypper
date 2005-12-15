/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* QueueItemBranch.cc
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

#include <zypp/solver/detail/QueueItemBranch.h>
#include <zypp/solver/detail/QueueItem.h>
#include <zypp/solver/detail/Resolver.h>

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////
      
      using namespace std;
      
      IMPL_DERIVED_POINTER(QueueItemBranch,QueueItem);
      
      //---------------------------------------------------------------------------
      
      string
      QueueItemBranch::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      string
      QueueItemBranch::toString ( const QueueItemBranch & item)
      {
          string res = "[Branch: ";
          if (!item._label.empty()) {
      	res += item._label;
          }
          res += "\n\t";
          res += QueueItem::toString(item._possible_items, "\n\t");
          res += "]";
          return res;
      }
      
      
      ostream &
      QueueItemBranch::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const QueueItemBranch & item)
      {
          return os << item.asString();
      }
      
      //---------------------------------------------------------------------------
      
      QueueItemBranch::QueueItemBranch (WorldPtr world)
          : QueueItem (QUEUE_ITEM_TYPE_BRANCH, world)
      {
      }
      
      
      QueueItemBranch::~QueueItemBranch()
      {
      }
      
      //---------------------------------------------------------------------------
      
      void
      QueueItemBranch::addItem (QueueItemPtr subitem)
      {
          assert (this != subitem);
      #if 0
          // We want to keep the list of possible items sorted for easy comparison later.
          for (QueueItemList::iterator pos = _possible_items.begin(); pos != _possible_items.end(); pos++) {
      
      	if ((*pos)->cmp(subitem) >= 0) {			// found a larger one
      	    _possible_items.insert (pos, subitem);		// insert before
      	    return;
      	}
          }
      #endif
          _possible_items.push_back (subitem);			// no larger found, subitem must be largest
      
          return;
      }
      
      
      bool
      QueueItemBranch::contains (QueueItemPtr possible_subbranch)
      {
          QueueItemBranchPtr branch = (QueueItemBranchPtr)possible_subbranch;
      
          if (branch == NULL
      	|| !branch->isBranch()) {
      	return false;
          }
      
      
          if (_possible_items.size() < branch->_possible_items.size()) {
      	return false;
          }
      
          QueueItemList::iterator iter = _possible_items.begin();
          QueueItemList::iterator iter_sub = branch->_possible_items.begin();
      
          /* For every item inside the possible sub-branch, look for a matching item
             in the branch.  If we can't find a match, fail.  (We can do this in one
             pass since the possible_items lists are sorted)
          */
          while (iter_sub != branch->_possible_items.end()) {
      
      	while (iter != _possible_items.end()
      	       && (*iter)->cmp (*iter_sub)) {
      	    iter++;
      	}
      
      	if (iter == _possible_items.end())
      	    return false;
      
      	iter++;
      	iter_sub++;
          }
      
          return true;
      }
      
      //---------------------------------------------------------------------------
      
      bool
      QueueItemBranch::process (ResolverContextPtr context, QueueItemList & qil)
      {
          if (getenv ("RC_SPEW")) fprintf (stderr, "QueueItemBranch::process(%s)\n", asString().c_str());
      
          QueueItemList live_branches;
          unsigned int branch_count;
          bool did_something = true;
      
          for (QueueItemList::const_iterator iter = _possible_items.begin(); iter != _possible_items.end(); iter++) {
      
      	QueueItemPtr item = *iter;
      
      	if (item->isSatisfied (context))
      	    goto finished;
      
      	/* Drop any useless branch items */
      	if (! item->isRedundant (context)) {
      	    live_branches.push_front (item);
      	}
          }
      
          branch_count = live_branches.size();
      
          if (branch_count == 0) {
      
      	/* Do nothing */
      
          } else if (branch_count == 1) {
      
      	/* If we just have one possible item, process it. */
      
      	QueueItemPtr item = live_branches.front();
      	did_something = item->process (context, qil);
      	
      	/* Set the item pointer to NULL inside of our original branch
      	   item, since our call to rc_queue_item_process is now
      	   responsible for freeing it. */
      
      	for (QueueItemList::iterator iter = _possible_items.begin(); iter != _possible_items.end(); iter++) {
      	    if (*iter == item) {
      		_possible_items.erase (iter);
      		break;
      	    }
      	}
      
          } else if (branch_count == _possible_items.size()) {
      
      	/* Nothing was eliminated, so just pass the branch through (and set it to
      	   NULL so that it won't get freed when we exit. */
      
      	qil.push_front (this);
      // FIXME: dont free	item = NULL;
      	did_something = false;
      
          } else {
      //fprintf (stderr, "QueueItemBranch::process rebranching\n");
      	QueueItemBranchPtr new_branch = new QueueItemBranch (world());
      	for (QueueItemList::const_iterator iter = live_branches.begin(); iter != live_branches.end(); iter++) {
      	    new_branch->addItem ((*iter)->copy());
      	}
      	qil.push_front (new_branch);
          }
          
       finished:
      //FIXME    rc_queue_item_free (item);
      
          return did_something;
      }
      
      
      int
      QueueItemBranch::cmp (constQueueItemPtr item) const
      {
          int cmp = this->compare (item);		// assures equal type
          if (cmp != 0)
      	return cmp;
      
          constQueueItemBranchPtr branch = item;
      
          /* First, sort by # of possible items. */
          cmp = CMP(_possible_items.size(), branch->_possible_items.size());
          if (cmp != 0)
              return cmp;
      
          /* We can do a by-item cmp since the possible items are kept in sorted order. */
          QueueItemList::const_iterator ia = _possible_items.begin();
          QueueItemList::const_iterator ib = branch->_possible_items.begin();
      
          while (ia != _possible_items.end() && ib != branch->_possible_items.end()) {
              if (*ia && *ib) {
                  cmp = (*ia)->cmp (*ib);
                  if (cmp != 0) {
                      return cmp;
      	    }
              }
              ia++;
              ib++;
          }
      
          /* Both lists should end at the same time, since we initially sorted on length. */
          assert (ia == _possible_items.end() && ib == branch->_possible_items.end());
      
          return 0;
      }
      
      
      QueueItemPtr
      QueueItemBranch::copy (void) const
      {
          QueueItemBranchPtr new_branch = new QueueItemBranch (world());
          ((QueueItemPtr)new_branch)->copy((constQueueItemPtr)this);
      
          for (QueueItemList::const_iterator iter = _possible_items.begin(); iter != _possible_items.end(); iter++) {
      	QueueItemPtr cpy = (*iter)->copy();
              new_branch->_possible_items.push_front (cpy);
          }
      
          return new_branch;
      }
      
      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

