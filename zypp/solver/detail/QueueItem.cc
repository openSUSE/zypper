/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* QueueItem.cc
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

#include "zypp/solver/detail/QueueItem.h"
#include "zypp/solver/detail/ResolverContext.h"


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

IMPL_PTR_TYPE(QueueItem);

//---------------------------------------------------------------------------

std::ostream &
QueueItem::dumpOn( std::ostream & os ) const
{
    return os << "<queueitem/>";
}


ostream&
operator<<( ostream & os, const QueueItemList & itemlist )
{
    for (QueueItemList::const_iterator iter = itemlist.begin(); iter != itemlist.end(); ++iter) {
	if (iter != itemlist.begin())
	    os << "," << endl << "\t";
	os << **iter;
    }
    return os;
}

//---------------------------------------------------------------------------

QueueItem::QueueItem (QueueItemType type, const ResPool & pool)
    : _type (type)
    , _pool (pool)
    , _priority (0)
    , _size (0)
{
}


QueueItem::~QueueItem()
{
}

//---------------------------------------------------------------------------

void
QueueItem::copy (const QueueItem *from)
{
    _priority = from->_priority;
    _size = from->_size;
    _pending_info = ResolverInfoList (from->_pending_info.begin(), from->_pending_info.end());
}


//---------------------------------------------------------------------------

void
QueueItem::addInfo (ResolverInfo_Ptr info)
{
    _pending_info.push_back (info);
}


void
QueueItem::logInfo (ResolverContext_Ptr context)
{
    for (ResolverInfoList::const_iterator iter = _pending_info.begin(); iter != _pending_info.end(); iter++) {
	context->addInfo (*iter);
    }
    _pending_info.clear();
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
