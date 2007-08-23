/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfoContainer.cc
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
#include <sstream>

#include "zypp/solver/detail/ResolverInfoContainer.h"
#include "zypp/solver/detail/Helper.h"

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

IMPL_PTR_TYPE(ResolverInfoContainer);

//---------------------------------------------------------------------------


std::ostream &
ResolverInfoContainer::dumpOn( std::ostream & os ) const
{
    ResolverInfo::dumpOn (os);

    os << "<resolverinfocontainer '";
    for (PoolItemList::const_iterator it = _item_list.begin(); it != _item_list.end(); ++it) {
	if (it != _item_list.begin()) os << ", ";
	os << ResolverInfo::toString (*it);
    }
    os << "'>";
    return os;
}

//---------------------------------------------------------------------------

ResolverInfoContainer::ResolverInfoContainer (ResolverInfoType type, PoolItem_Ref item, int priority, PoolItem_Ref child)
    : ResolverInfo (type, item, priority)
{
    if (child)
	_item_list.push_back (child);
}


ResolverInfoContainer::~ResolverInfoContainer ()
{
}

//---------------------------------------------------------------------------

bool
ResolverInfoContainer::merge (ResolverInfoContainer_Ptr to_be_merged)
{
    bool res;

    res = ((ResolverInfo_Ptr)this)->merge ((ResolverInfo_Ptr)to_be_merged);
    if (!res) return res;

    typedef std::map<PoolItem_Ref, bool> SeenTable;
    SeenTable seen_packages;

    for (PoolItemList::const_iterator iter = _item_list.begin(); iter != _item_list.end(); iter++) {
	seen_packages[*iter] = true;
    }

    PoolItemList rl = to_be_merged->items();
    for (PoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	SeenTable::const_iterator pos = seen_packages.find(*iter);
	if (pos == seen_packages.end()) {
	    _item_list.push_front (*iter);
	    seen_packages[*iter] = true;
	}
    }

    return true;
}


void
ResolverInfoContainer::copy (ResolverInfoContainer_constPtr from)
{
    ((ResolverInfo_Ptr)this)->copy(from);

    for (PoolItemList::const_iterator iter = from->_item_list.begin(); iter != from->_item_list.end(); iter++) {
	_item_list.push_back (*iter);
    }
}


ResolverInfo_Ptr
ResolverInfoContainer::copy (void) const
{
    ResolverInfoContainer_Ptr cpy = new ResolverInfoContainer(type(), affected(), priority());

    cpy->copy (this);

    return cpy;
}

//---------------------------------------------------------------------------

string
ResolverInfoContainer::itemsToString (const bool names_only) const
{
    ostringstream res;

    if (_item_list.empty())
	return "";

    if (names_only)
    {
	if (_item_list.size() > 1) res << " [";
	for (PoolItemList::const_iterator iter = _item_list.begin();
	     iter != _item_list.end(); ++iter)
	{
	    if (iter != _item_list.begin())
		res << ", ";
	    res << (*iter)->name();
	}
	if (_item_list.size() > 1) res << "]";
    }
    else
    {
	if (_item_list.size() == 1) {
	    res << ResolverInfo::toString (*_item_list.begin());
	} else {
	    // one line for each entry
	    for (PoolItemList::const_iterator iter = _item_list.begin();
		 iter != _item_list.end(); iter++)
	    {
		res << "\n- ";	    
		res << ResolverInfo::toString (*iter);
	    }
	}
    }

    return res.str();
}


bool
ResolverInfoContainer::mentions (PoolItem_Ref item) const
{
    if (isAbout(item))
	return true;

    // Search item_list for any mention of the item.

    for (PoolItemList::const_iterator iter = _item_list.begin(); iter != _item_list.end(); iter++) {
	if ((*iter)->name() == item->name()
	    && (*iter)->kind() == item->kind()) {
	    return true;
	}
    }
    
    return false;
}


void
ResolverInfoContainer::addRelatedPoolItem (PoolItem_Ref item)
{
    if (item && !mentions(item)) {
	_item_list.push_front (item);
    }
}


void
ResolverInfoContainer::addRelatedPoolItemList (const PoolItemList & items)
{
    for (PoolItemList::const_iterator iter = items.begin(); iter != items.end(); iter++) {
	_item_list.push_front (*iter);
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

