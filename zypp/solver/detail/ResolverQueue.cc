/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverQueue.cc
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

#include <map>

#include "zypp/base/String.h"
#include "zypp/solver/detail/ResolverQueue.h"
#include "zypp/solver/detail/QueueItemBranch.h"
#include "zypp/solver/detail/QueueItemConflict.h"
#include "zypp/solver/detail/QueueItemGroup.h"
#include "zypp/solver/detail/QueueItemInstall.h"
#include "zypp/solver/detail/QueueItemRequire.h"
#include "zypp/solver/detail/QueueItemUninstall.h"
#include "zypp/solver/detail/ResolverContext.h"
#include "zypp/CapSet.h"
#include "zypp/base/Logger.h"

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

      IMPL_PTR_TYPE(ResolverQueue);

      //---------------------------------------------------------------------------


      //---------------------------------------------------------------------------

      string
      ResolverQueue::asString ( void ) const
      {
          return toString (*this);
      }


      string
      ResolverQueue::toString ( const ResolverQueue & resolverqueue )
      {
          string res;

          res += str::form ("Context [%p]", (const void *)resolverqueue._context.get());
          res +=  ", Items:\n\t";
          res += QueueItem::toString (resolverqueue._items, ",\n\t");

          return res;
      }


      ostream &
      ResolverQueue::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }


      ostream&
      operator<<( ostream& os, const ResolverQueue & resolverqueue)
      {
          return os << resolverqueue.asString();
      }

      //---------------------------------------------------------------------------

      ResolverQueue::ResolverQueue (ResolverContext_Ptr context)
          : _context (context)
      {
          if (context == NULL)
      	_context = new ResolverContext();
      }


      ResolverQueue::~ResolverQueue()
      {
      }

      //---------------------------------------------------------------------------

      void
      ResolverQueue::addResItemToInstall (ResItem_constPtr resItem)
      {
          QueueItemInstall_Ptr item;

          if (_context->resItemIsPresent (resItem)) {
      	printf ("%s is already installed", resItem->asString().c_str());
      	return;
          }

          item = new QueueItemInstall (_context->world(), resItem);
          item->setExplicitlyRequested ();

          addItem (item);
      }


      void
      ResolverQueue::addResItemToRemove (ResItem_constPtr resItem, bool remove_only_mode)
      {
          QueueItemUninstall_Ptr item;

          if (_context->resItemIsAbsent (resItem))
      	return;

          item = new QueueItemUninstall (_context->world(), resItem, "user request");
          if (remove_only_mode)
      	item->setRemoveOnly ();

          item->setExplicitlyRequested ();

          addItem (item);
      }


      void
      ResolverQueue::addResItemToVerify (ResItem_constPtr resItem)
      {
          World_Ptr world;

          world = _context->world ();

          CapSet requires = resItem->requires();
          for (CapSet::const_iterator iter = requires.begin(); iter != requires.end(); iter++) {
      	QueueItemRequire_Ptr item = new QueueItemRequire (world, *iter);
      	item->addResItem (resItem);
      	addItem (item);
          }

          CapSet conflicts = resItem->conflicts();
          for (CapSet::const_iterator iter = conflicts.begin(); iter != conflicts.end(); iter++) {
      	QueueItemConflict_Ptr item = new QueueItemConflict (world, *iter, resItem);
      	addItem (item);
          }
      }


      void
      ResolverQueue::addExtraDependency (const Capability & dep)
      {
          QueueItemRequire_Ptr item = new QueueItemRequire (_context->world(), dep);
          addItem (item);
      }


      void
      ResolverQueue::addExtraConflict (const Capability & dep)
      {
          QueueItemConflict_Ptr item = new QueueItemConflict (_context->world(), dep, NULL);
          addItem (item);
      }


      void
      ResolverQueue::addItem (QueueItem_Ptr item)
      {
          _items.push_front (item);
      }


      bool
      ResolverQueue::isInvalid ()
      {
          return _context->isInvalid();
      }


      bool
      ResolverQueue::containsOnlyBranches ()
      {
          for (QueueItemList::const_iterator iter = _items.begin(); iter != _items.end(); iter++) {
      	if (!(*iter)->isBranch())
      	    return false;
          }

          return true;
      }

      //---------------------------------------------------------------------------

      static int
      itemlist_max_priority (const QueueItemList & qil)
      {
          int max_priority = -1;
          for (QueueItemList::const_iterator iter = qil.begin(); iter != qil.end(); iter++) {
      	if ((*iter)->priority() > max_priority) {
      	    max_priority = (*iter)->priority();
      	}
          }

          return max_priority;
      }



      bool
      ResolverQueue::processOnce ()
      {
          QueueItemList new_items;
          int max_priority;
          bool did_something = false;

          _DBG("QUEUE_SPEW") << "ResolverQueue::processOnce(" << asString() <<", " << (int) _items.size() << " items" << endl;
          while ( (max_priority = itemlist_max_priority (_items)) >= 0
      	    && _context->isValid () ) {

      	bool did_something_recently = false;

	_DBG("QUEUE_SPEW") << "ResolverQueue::processOnce() inside loop" << endl;
      	for (QueueItemList::iterator iter = _items.begin(); iter != _items.end() && _context->isValid();) {
      	    QueueItem_Ptr item = *iter;
      	    _DBG("QUEUE_SPEW") <<  "=====> 1st pass: [" << item->asString() << "]" << endl;
      	    QueueItemList::iterator next = iter; next++;
      	    if (item && item->priority() == max_priority) {
      		if (item->process (_context, new_items)) {
      		    did_something_recently = true;
      		}
      		_items.erase (iter);
      	    }
      	    iter = next;
      	}

      	if (did_something_recently) {
      	    did_something = true;
      	}
          }

          _items = new_items;
          _DBG("QUEUE_SPEW") <<  (int) _items.size() << " items after first pass" << endl;

          /*
             Now make a second pass over the queue, removing any super-branches.
             (If one branch contains all of the possible items of another branch,
             the larger branch can be dropped.
          */

          _XXX("QUEUE_SPEW") <<  "ResolverQueue::processOnce() second pass" << endl;
          for (QueueItemList::iterator iter = _items.begin(); iter != _items.end();) {
      	QueueItemList::iterator next = iter; next++;
      	QueueItem_Ptr item = *iter;

      	_DBG("QUEUE_SPEW") <<  "=====> 2nd pass: [" << item->asString() << "]" << endl;
      	if (item->isBranch()) {
      	    _DBG("QUEUE_SPEW") << "ResolverQueue::processOnce() is branch" << endl;
      	    QueueItemBranch_Ptr branch = dynamic_pointer_cast<QueueItemBranch>(item);
      	    for (QueueItemList::const_iterator iter2 = _items.begin(); iter2 != _items.end(); iter2++) {
      		_DBG("QUEUE_SPEW") << "Compare branch with [" << (*iter2)->asString() << "]" << endl;
      		if (iter != iter2
      		    && branch->contains (*iter2)) {
      		    _DBG("QUEUE_SPEW") << "Contained within, removing" << endl;
      		    _items.erase (iter);
      		    break;
      		}
      	    }
      	}
      	iter = next;
          }
	  if (did_something)
	      _DBG("QUEUE_SPEW") <<  "did somesthing: " << (int)_items.size() << " items" << endl;
	  else
	      _DBG("QUEUE_SPEW") <<  "did nothing: " << (int)_items.size() << " items" << endl;	      

          return did_something;
      }


      void
      ResolverQueue::process ()
      {
	  _DBG("RC_SPEW") << "----- Processing -----" << endl;
	  spew ();

          while (_context->isValid ()
		 && ! isEmpty ()
		 && processOnce ()) {
	      /* all of the work is in the conditional! */
	      spew ();
          }
      }


      //---------------------------------------------------------------------------

      static ResolverQueue_Ptr
      copy_queue_except_for_branch (ResolverQueue_Ptr queue, QueueItem_Ptr branch_item, QueueItem_Ptr subitem)
      {
          ResolverContext_Ptr new_context;
          ResolverQueue_Ptr new_queue;

          new_context = new ResolverContext (queue->context());
          new_queue = new ResolverQueue (new_context);

          QueueItemList qil = queue->items();
          for (QueueItemList::const_iterator iter = qil.begin(); iter != qil.end(); iter++) {
      	QueueItem_Ptr item = *iter;
      	QueueItem_Ptr new_item;

      	if (item == branch_item) {
      	    new_item = subitem->copy ();

      	    if (new_item->isInstall()) {
      		QueueItemInstall_Ptr install_item = dynamic_pointer_cast<QueueItemInstall>(new_item);

      		/* Penalties are negative priorities */
      		int penalty;
      		penalty = - queue->context()->getChannelPriority (install_item->resItem()->channel());

      		install_item->setOtherPenalty (penalty);
      	    }

      	} else {

      	    new_item = item->copy ();

      	}

      	new_queue->addItem (new_item);
          }

          return new_queue;
      }


      void
      ResolverQueue::splitFirstBranch (ResolverQueueList & new_queues, ResolverQueueList & deferred_queues)
      {
          QueueItemBranch_Ptr first_branch = NULL;
          typedef std::map <QueueItem_Ptr, QueueItem_Ptr> DeferTable;
          DeferTable to_defer;

          for (QueueItemList::const_iterator iter = _items.begin(); iter != _items.end() && first_branch == NULL; iter++) {
      	QueueItem_Ptr item = *iter;
      	if (item->isBranch()) {
      	    first_branch = dynamic_pointer_cast<QueueItemBranch>(item);
      	}
          }

          if (first_branch == NULL)
      	return;

          /*
             Check for deferrable items: if we have two install items where the to-be-installed
             resItems have the same name, then we will defer the lower-priority install if
             one of the following is true:
             (1) Both resItems have the same version
             (2) The lower-priority channel is a previous version.
          */

          QueueItemList possible_items = first_branch->possibleItems();
          for (QueueItemList::const_iterator iter = possible_items.begin(); iter != possible_items.end(); iter++) {
      	QueueItemList::const_iterator iter2 = iter;
      	for (iter2++; iter2 != possible_items.end(); iter2++) {
      	    QueueItem_Ptr item = *iter;
      	    QueueItem_Ptr item2 = *iter2;

      	    if (item->isInstall() && item2->isInstall()) {
      		ResItem_constPtr r = (dynamic_pointer_cast<QueueItemInstall>(item))->resItem();
      		ResItem_constPtr r2 = (dynamic_pointer_cast<QueueItemInstall>(item2))->resItem();
      		Channel_constPtr channel = r->channel();
      		Channel_constPtr channel2 = r2->channel();
      		int priority, priority2;

      		priority = channel->getPriority (channel->isSubscribed());
      		priority2 = channel2->getPriority (channel2->isSubscribed());

      		if (priority != priority2 && r->name() == r2->name()) {
      		    if (r->version() == r2->version()
      			|| (priority < priority2 && ResItem::compare (r, r2) < 0)
      			|| (priority > priority2 && ResItem::compare (r, r2) > 0)) {

      			if (priority < priority2)
      			    to_defer[item] = item;
      			else /* if (priority > priority2) */
      			    to_defer[item2] = item2;
      		    }
      		}
      	    }
      	}
          }


          for (QueueItemList::const_iterator iter = possible_items.begin(); iter != possible_items.end(); iter++) {
      	ResolverQueue_Ptr new_queue;
      	QueueItem_Ptr new_item = *iter;

      	new_queue = copy_queue_except_for_branch (this, (QueueItem_Ptr) first_branch, new_item);

      	DeferTable::const_iterator pos = to_defer.find (new_item);
      	if (pos != to_defer.end()) {
      	    deferred_queues.push_back (new_queue);
      	} else {
      	    new_queues.push_back (new_queue);
      	}
          }

      }


      void
      ResolverQueue::spew ()
      {
          _DBG("RC_SPEW") << "Resolver Queue: " << (_context->isInvalid() ? "INVALID" : "") << endl;

          if (_items.empty()) {

	      _DBG("RC_SPEW") <<  "  (empty)" << endl;

          } else {
	      for (QueueItemList::const_iterator iter = _items.begin(); iter != _items.end(); iter++) {
		  _DBG("RC_SPEW") << "  " << (*iter)->asString() << endl;
	      }

          }

          _DBG("RC_SPEW") << endl;
          fflush (stdout);
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

