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
#include "zypp/solver/detail/QueueItemEstablish.h"
#include "zypp/solver/detail/QueueItemGroup.h"
#include "zypp/solver/detail/QueueItemInstall.h"
#include "zypp/solver/detail/QueueItemRequire.h"
#include "zypp/solver/detail/QueueItemUninstall.h"
#include "zypp/solver/detail/ResolverContext.h"
#include "zypp/Capabilities.h"
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

ostream&
operator<<( ostream& os, const ResolverQueue & resolverqueue)
{
    os << str::form ("Context [%p]", (const void *)resolverqueue._context.get());
    os <<  ", " << resolverqueue._qitems.size() << " Items:" << endl << "\t";
    os << resolverqueue._qitems;
    return os;
}

//---------------------------------------------------------------------------

ResolverQueue::ResolverQueue (const ResPool & pool, const Arch & arch, ResolverContext_Ptr context)
    : _context (context)
{
    _XDEBUG("ResolverQueue::ResolverQueue(pool,... )");
    if (context == NULL)
	_context = new ResolverContext(pool, arch);
}


ResolverQueue::~ResolverQueue()
{
}

//---------------------------------------------------------------------------

void
ResolverQueue::addPoolItemToInstall (PoolItem_Ref poolItem)
{
    QueueItemInstall_Ptr qitem;

    if (_context->isPresent (poolItem)
	&& poolItem.status().staysInstalled()) {
	WAR << poolItem << " is already installed" << endl;
	return;
    }

    qitem = new QueueItemInstall (_context->pool(), poolItem, poolItem.status().isSoftInstall());
    poolItem.status().setSoftInstall(false);
    qitem->setExplicitlyRequested ();

    addItem (qitem);
}


void
ResolverQueue::addPoolItemToEstablish (PoolItem_Ref poolItem)
{
    QueueItemEstablish_Ptr qitem;

    qitem = new QueueItemEstablish (_context->pool(), poolItem);
    qitem->setExplicitlyRequested ();

    addItem (qitem);
}


void
ResolverQueue::addPoolItemToRemove (PoolItem_Ref poolItem, bool remove_only_mode)
{
    QueueItemUninstall_Ptr qitem;

    if (_context->isAbsent (poolItem))
	return;

    qitem = new QueueItemUninstall (_context->pool(), poolItem, QueueItemUninstall::EXPLICIT, poolItem.status().isSoftUninstall());
    poolItem.status().setSoftUninstall(false);
    if (remove_only_mode)
	qitem->setRemoveOnly ();

    qitem->setExplicitlyRequested ();

    addItem (qitem);
}


void
ResolverQueue::addPoolItemToVerify (PoolItem_Ref poolItem)
{
    if (poolItem.status().staysInstalled()
	|| poolItem.status().isToBeInstalled()) {
	// regarding only items which will on the system after commit
	Capabilities requires = poolItem->dep (Dep::REQUIRES);
	for (Capabilities::const_iterator iter = requires.begin(); iter != requires.end(); ++iter) {
	    QueueItemRequire_Ptr qitem = new QueueItemRequire (_context->pool(), *iter);
	    qitem->addPoolItem (poolItem);
	    addItem (qitem);
	}

	Capabilities conflicts = poolItem->dep (Dep::CONFLICTS);
	for (Capabilities::const_iterator iter = conflicts.begin(); iter != conflicts.end(); ++iter) {
	    QueueItemConflict_Ptr qitem = new QueueItemConflict (_context->pool(), *iter, poolItem);
	    addItem (qitem);
	}
    }
}


void
ResolverQueue::addExtraCapability (const Capability & dep)
{
    QueueItemRequire_Ptr qitem = new QueueItemRequire (_context->pool(), dep);
    addItem (qitem);
}


void
ResolverQueue::addExtraConflict (const Capability & dep)
{
    QueueItemConflict_Ptr qitem = new QueueItemConflict (_context->pool(), dep, PoolItem_Ref());
    qitem->setExplicitlyRequested();
    addItem (qitem);
}


void
ResolverQueue::addItem (QueueItem_Ptr qitem)
{
    _qitems.push_back(qitem);
}


bool
ResolverQueue::isInvalid ()
{
    return _context->isInvalid() || _context->askUser();
}


bool
ResolverQueue::containsOnlyBranches ()
{
    for (QueueItemList::const_iterator iter = _qitems.begin(); iter != _qitems.end(); ++iter) {
	if (!(*iter)->isBranch())
	    return false;
    }

    return true;
}

//---------------------------------------------------------------------------

static int
qitemlist_max_priority (const QueueItemList & qil)
{
    int max_priority = -1;
    for (QueueItemList::const_iterator iter = qil.begin(); iter != qil.end(); ++iter) {
	if ((*iter)->priority() > max_priority) {
	    max_priority = (*iter)->priority();
	}
    }

    return max_priority;
}



bool
ResolverQueue::processOnce ()
{
    QueueItemList new_qitems;
    int max_priority;
    bool did_something = false;

    _XDEBUG("ResolverQueue::processOnce()" << (int) _qitems.size() << " items");
    while ( (max_priority = qitemlist_max_priority (_qitems)) >= 0
	    && _context->isValid () ) {

	bool did_something_recently = false;

	for (QueueItemList::iterator iter = _qitems.begin(); iter != _qitems.end() && _context->isValid();) {
	    QueueItem_Ptr qitem = *iter;
	    _XDEBUG( "=====> 1st pass: [" << *qitem << "]");
	    QueueItemList::iterator next = iter; ++next;
	    if (qitem && qitem->priority() == max_priority) {
		if (qitem->process (_qitems, _context, new_qitems)) {
		    did_something_recently = true;
		}
		_qitems.erase (iter);
	    }
	    iter = next;
	}

	if (did_something_recently) {
	    did_something = true;
	}
    }

    _qitems = new_qitems;
    _XDEBUG(_qitems.size() << " qitems after first pass");

    /*
       Now make a second pass over the queue, removing any super-branches.
       (If one branch contains all of the possible items of another branch,
       the larger branch can be dropped.
    */

    for (QueueItemList::iterator iter = _qitems.begin(); iter != _qitems.end();) {
	QueueItemList::iterator next = iter; ++next;
	QueueItem_Ptr qitem = *iter;

	_XDEBUG( "=====> 2nd pass: [" << *qitem << "]");
	if (qitem->isBranch()) {
	    _XDEBUG("ResolverQueue::processOnce() is branch");
	    QueueItemBranch_Ptr branch = dynamic_pointer_cast<QueueItemBranch>(qitem);
	    for (QueueItemList::const_iterator iter2 = _qitems.begin(); iter2 != _qitems.end(); ++iter2) {
		_XDEBUG("Compare branch with [" << *(*iter2) << "]");
		if (iter != iter2
		    && branch->contains (*iter2)) {
		    _XDEBUG("Contained within, removing");
		    _qitems.erase (iter);
		    break;
		}
	    }
	}
	iter = next;
    }
    if (did_something) {
      _XDEBUG( "did something: " << _qitems.size() << " qitems");
    }
    else {
      _XDEBUG( "did nothing: " << _qitems.size() << " qitems"); 
    }

    return did_something;
}


void
ResolverQueue::process ()
{
    _XDEBUG("----- Processing -----");
    spew ();

    while (_context->isValid ()
		 && ! isEmpty ()
		 && processOnce ()) {
	      /* all of the work is in the conditional! */
	      spew ();
    }
}


//---------------------------------------------------------------------------

ResolverQueue_Ptr
ResolverQueue::copy_queue_except_for_branch (QueueItem_Ptr branch_qitem, QueueItem_Ptr subqitem) const
{
    ResolverContext_Ptr new_context;
    ResolverQueue_Ptr new_queue;
    _XDEBUG("copy_queue_except_for_branch");
    new_context = new ResolverContext (_context->pool(), _context->architecture(), _context);
      
    new_queue = new ResolverQueue (new_context->pool(), new_context->architecture(), new_context);

    QueueItemList qil = _qitems;
    for (QueueItemList::const_iterator iter = qil.begin(); iter != qil.end(); ++iter) {
	QueueItem_Ptr qitem = *iter;
	QueueItem_Ptr new_qitem;

	if (qitem == branch_qitem) {
	    new_qitem = subqitem->copy ();

	    if (new_qitem->isInstall()) {
		QueueItemInstall_Ptr install_qitem = dynamic_pointer_cast<QueueItemInstall>(new_qitem);

		/* Penalties are negative priorities */
#warning FIX priorities                
		int penalty = 0;
		Repository repo = install_qitem->item()->repository();
		//penalty = - src.priority();
		install_qitem->setOtherPenalty (penalty);
	    }

	} else {

	    new_qitem = qitem->copy ();

	}

	new_queue->addItem (new_qitem);
    }

    return new_queue;
}


void
ResolverQueue::splitFirstBranch (ResolverQueueList & new_queues, ResolverQueueList & deferred_queues)
{
    QueueItemBranch_Ptr first_branch = NULL;
    typedef std::map <QueueItem_Ptr, QueueItem_Ptr> DeferTable;
    DeferTable to_defer;

    for (QueueItemList::const_iterator iter = _qitems.begin(); iter != _qitems.end() && first_branch == NULL; ++iter) {
	QueueItem_Ptr qitem = *iter;
	if (qitem->isBranch()) {
	    first_branch = dynamic_pointer_cast<QueueItemBranch>(qitem);
	}
    }

    if (first_branch == NULL)
	return;

    QueueItemList possible_qitems = first_branch->possibleQItems();

    /*
       Check for deferrable qitems: if we have two install qitems where the to-be-installed
       poolItems have the same name, then we will defer the lower-priority install if
       one of the following is true:
       (1) Both poolItems have the same version
       (2) The lower-priority channel is a previous version.
    */

    for (QueueItemList::const_iterator iter = possible_qitems.begin(); iter != possible_qitems.end(); ++iter) {
	QueueItemList::const_iterator iter2 = iter;
	for (iter2++; iter2 != possible_qitems.end(); iter2++) {
	    QueueItem_Ptr qitem = *iter;
	    QueueItem_Ptr qitem2 = *iter2;

	    if (qitem->isInstall() && qitem2->isInstall()) {
		PoolItem_Ref r = (dynamic_pointer_cast<QueueItemInstall>(qitem))->item();
		PoolItem_Ref r2 = (dynamic_pointer_cast<QueueItemInstall>(qitem2))->item();
		Repository repo1 = r->repository();
		Repository repo2 = r2->repository();
		int priority, priority2;

		priority = _context->getRepoPriority( repo1 );
		priority2 = _context->getRepoPriority( repo2 );

		if (priority != priority2 && r->name() == r2->name()) {
		    if (r->edition().compare(r2->edition()) == 0
			|| (priority < priority2 && r->edition().compare(r2->edition()) < 0)
			|| (priority > priority2 && r->edition().compare(r2->edition()) > 0))
		    {
			if (priority < priority2)
			    to_defer[qitem] = qitem;
			else /* if (priority > priority2) */
			    to_defer[qitem2] = qitem2;
		    }
		}
	    }
	}
    }

    for (QueueItemList::const_iterator iter = possible_qitems.begin(); iter != possible_qitems.end(); ++iter) {
	ResolverQueue_Ptr new_queue;
	QueueItem_Ptr new_qitem = *iter;

	new_queue = copy_queue_except_for_branch ((QueueItem_Ptr) first_branch, new_qitem);

	DeferTable::const_iterator pos = to_defer.find (new_qitem);
	if (pos != to_defer.end()) {
	    deferred_queues.push_back(new_queue);
	} else {
	    new_queues.push_back(new_queue);
	}
    }

}


void
ResolverQueue::spew ()
{
    _XDEBUG( "Resolver Queue: " << (_context->isInvalid() ? "INVALID" : "") );

    if (_qitems.empty()) {

	_XDEBUG( "  (empty)" );

    } else {
	for (QueueItemList::const_iterator iter = _qitems.begin(); iter != _qitems.end(); ++iter) {
	    _XDEBUG( "  " << *(*iter) );
	}

    }
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

