/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* QueueItemGroup.cc
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

#include "zypp/solver/detail/QueueItemGroup.h"
#include "zypp/solver/detail/QueueItem.h"
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

IMPL_PTR_TYPE(QueueItemGroup);

//---------------------------------------------------------------------------

string
QueueItemGroup::asString ( void ) const
{
    return toString (*this);
}


string
QueueItemGroup::toString ( const QueueItemGroup & item)
{
    string ret = "[Group: ";
    ret += QueueItem::toString(item._subitems);
    ret += "]";
    return ret;
}


ostream &
QueueItemGroup::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const QueueItemGroup & item)
{
    return os << item.asString();
}

//---------------------------------------------------------------------------

QueueItemGroup::QueueItemGroup (World_Ptr world)
    : QueueItem (QUEUE_ITEM_TYPE_GROUP, world)
{
}


QueueItemGroup::~QueueItemGroup()
{
}

//---------------------------------------------------------------------------

bool
QueueItemGroup::process (ResolverContext_Ptr context, QueueItemList & new_items)
{
    _DBG("RC_SPEW") << "QueueItemGroup::process" << endl;

    bool did_something = false;

    // Just move all of the group's subitems onto the new_items list.

    for (QueueItemList::const_iterator iter = _subitems.begin(); iter != _subitems.end(); iter++) {
	new_items.push_front (*iter);
	did_something = true;
    }

    _subitems.clear();

// FIXME: delete self

    return did_something;
}


QueueItem_Ptr
QueueItemGroup::copy (void) const
{
    QueueItemGroup_Ptr new_group = new QueueItemGroup (world());
    new_group->QueueItem::copy(this);

    for (QueueItemList::const_iterator iter = _subitems.begin(); iter != _subitems.end(); iter++) {
	new_group->_subitems.push_back ((*iter)->copy());
    }
    return new_group;
}


int
QueueItemGroup::cmp (QueueItem_constPtr item) const
{
    int cmp = this->compare (item);		// assures equal type
    if (cmp != 0)
	return cmp;

    QueueItemGroup_constPtr group = dynamic_pointer_cast<const QueueItemGroup>(item);

    // First, sort by # of subitems

    cmp = CMP(_subitems.size(), group->_subitems.size());
    if (cmp)
        return cmp;

    // We can do a by-item cmp since the possible items are kept in sorted order.
    QueueItemList::const_iterator iter2;
    for (QueueItemList::const_iterator iter = _subitems.begin(), iter2 = group->_subitems.begin();
	 iter != _subitems.end() && iter2 != group->_subitems.end(); iter++, iter2++) {
	cmp = (*iter)->cmp (*iter2);
	if (cmp) {
	    return cmp;
	}
    }

    return 0;
}


void
QueueItemGroup::addItem (QueueItem_Ptr subitem)
{
    // We need to keep the list sorted for comparison purposes.
    _subitems.push_back (subitem);
// FIXME    _subitems.sort(cmp)
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

