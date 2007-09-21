/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
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

#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/QueueItemBranch.h"
#include "zypp/solver/detail/QueueItem.h"
#include "zypp/solver/detail/Resolver.h"
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

IMPL_PTR_TYPE(QueueItemBranch);

//---------------------------------------------------------------------------

std::ostream &
QueueItemBranch::dumpOn( std::ostream & os ) const
{
    os << "[Branch: ";
    if (!_label.empty()) {
	os << _label;
    }
    os << ", " << _possible_qitems.size() << " items." << endl << "\t";
    os << _possible_qitems << endl << "\t";
    os << "]";
    return os;
}

//---------------------------------------------------------------------------

QueueItemBranch::QueueItemBranch (const ResPool & pool)
    : QueueItem (QUEUE_ITEM_TYPE_BRANCH, pool)
{
}


QueueItemBranch::~QueueItemBranch()
{
}

//---------------------------------------------------------------------------

void
QueueItemBranch::addItem (QueueItem_Ptr subitem)
{
    assert (static_cast<QueueItem*>(this) != subitem);
#if 0
    // We want to keep the list of possible items sorted for easy comparison later.
    for (QueueItemList::iterator pos = _possible_qitems.begin(); pos != _possible_qitems.end(); pos++) {

	if ((*pos)->cmp(subitem) >= 0) {			// found a larger one
	    _possible_qitems.insert (pos, subitem);		// insert before
	    return;
	}
    }
#endif
    _possible_qitems.push_back (subitem);			// no larger found, subitem must be largest

    return;
}


bool
QueueItemBranch::contains (QueueItem_Ptr possible_subbranch)
{
    QueueItemBranch_Ptr branch = dynamic_pointer_cast<QueueItemBranch>(possible_subbranch);

    if (branch == NULL
	|| !branch->isBranch()) {
	return false;
    }

    if (_possible_qitems.size() < branch->_possible_qitems.size()) {
	return false;
    }

    QueueItemList::iterator iter = _possible_qitems.begin();
    QueueItemList::iterator iter_sub = branch->_possible_qitems.begin();

    /* For every item inside the possible sub-branch, look for a matching item
       in the branch.  If we can't find a match, fail.  (We can do this in one
       pass since the possible_qitems lists are sorted)
    */
    while (iter_sub != branch->_possible_qitems.end()) {

	while (iter != _possible_qitems.end()
	       && (*iter)->cmp (*iter_sub)) {
	    iter++;
	}

	if (iter == _possible_qitems.end())
	    return false;

	iter++;
	iter_sub++;
    }

    return true;
}

//---------------------------------------------------------------------------

bool
QueueItemBranch::process (const QueueItemList & mainQueue, ResolverContext_Ptr context, QueueItemList & qil)
{
    _XDEBUG("QueueItemBranch::process(" << *this << ")");

    QueueItemList live_branches;
    unsigned int branch_count;
    bool did_something = true;

    for (QueueItemList::const_iterator iter = _possible_qitems.begin(); iter != _possible_qitems.end(); iter++) {

	QueueItem_Ptr item = *iter;
	_XDEBUG("_possible_qitem " << *item);
	if (item->isSatisfied (context)) {
	    _XDEBUG("is satisfied");
	    goto finished;
	}

	/* Drop any useless branch items */
	if (! item->isRedundant (context)) {
	    _XDEBUG("not redundant");
	    live_branches.push_front (item);
	}
    }

    branch_count = live_branches.size();
    _XDEBUG("branch_count " << branch_count);

    if (branch_count == 0) {

	/* Do nothing */

    } else if (branch_count == 1) {

	/* If we just have one possible item, process it. */

	QueueItem_Ptr item = live_branches.front();
	did_something = item->process (mainQueue, context, qil);

	/* Set the item pointer to NULL inside of our original branch
	   item, since our call to rc_queue_item_process is now
	   responsible for freeing it. */

	for (QueueItemList::iterator iter = _possible_qitems.begin(); iter != _possible_qitems.end(); iter++) {
	    if (*iter == item) {
		_possible_qitems.erase (iter);
		break;
	    }
	}

    } else if (branch_count == _possible_qitems.size()) {
	_XDEBUG("Nothing was eliminated");
	/* Nothing was eliminated, so just pass the branch through. */

	qil.push_front (this);
	did_something = false;

    } else {
	_XDEBUG("rebranching");
	QueueItemBranch_Ptr new_branch = new QueueItemBranch (pool());
	for (QueueItemList::const_iterator iter = live_branches.begin(); iter != live_branches.end(); iter++) {
	    new_branch->addItem ((*iter)->copy());
	}
	qil.push_front (new_branch);
    }

 finished:

    return did_something;
}


int
QueueItemBranch::cmp (QueueItem_constPtr item) const
{
    int cmp = this->compare (item);		// assures equal type
    if (cmp != 0)
	return cmp;

    QueueItemBranch_constPtr branch = dynamic_pointer_cast<const QueueItemBranch>(item);

    /* First, sort by # of possible items. */
    cmp = CMP(_possible_qitems.size(), branch->_possible_qitems.size());
    if (cmp != 0)
        return cmp;

    /* We can do a by-item cmp since the possible items are kept in sorted order. */
    QueueItemList::const_iterator ia = _possible_qitems.begin();
    QueueItemList::const_iterator ib = branch->_possible_qitems.begin();

    while (ia != _possible_qitems.end() && ib != branch->_possible_qitems.end()) {
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
    assert (ia == _possible_qitems.end() && ib == branch->_possible_qitems.end());

    return 0;
}

/***/
QueueItem_Ptr
QueueItemBranch::copy (void) const
{
    QueueItemBranch_Ptr new_branch = new QueueItemBranch (pool());
    new_branch->QueueItem::copy(this);
    for (QueueItemList::const_iterator iter = _possible_qitems.begin(); iter != _possible_qitems.end(); iter++) {
	QueueItem_Ptr cpy = (*iter)->copy();
        new_branch->_possible_qitems.push_front (cpy);
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

