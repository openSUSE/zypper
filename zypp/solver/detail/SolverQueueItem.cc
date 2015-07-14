/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* SolverQueueItem.cc
 *
 * Copyright (C) 2008 SUSE Linux Products GmbH
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
extern "C"
{
#include <solv/solver.h>
}

#define ZYPP_USE_RESOLVER_INTERNALS

#include "zypp/base/Logger.h"
#include "zypp/solver/detail/SolverQueueItem.h"

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

IMPL_PTR_TYPE(SolverQueueItem);

//---------------------------------------------------------------------------

std::ostream &
SolverQueueItem::dumpOn( std::ostream & os ) const
{
    switch (_type) {
      case QUEUE_ITEM_TYPE_UNKNOWN       :	os << "unknown"; break;
      case QUEUE_ITEM_TYPE_UPDATE        :	os << "update"; break;
      case QUEUE_ITEM_TYPE_LOCK          :	os << "lock"; break;
      case QUEUE_ITEM_TYPE_INSTALL       :	os << "install"; break;
      case QUEUE_ITEM_TYPE_DELETE        :	os << "delete"; break;
      case QUEUE_ITEM_TYPE_INSTALL_ONE_OF:	os << "install one of"; break;
      default: os << "?solverqueueitem?"; break;
    }
    return os;
}


ostream&
operator<<( ostream & os, const SolverQueueItemList & itemlist )
{
    for (SolverQueueItemList::const_iterator iter = itemlist.begin(); iter != itemlist.end(); ++iter) {
	if (iter != itemlist.begin())
	    os << "," << endl << "\t";
	os << **iter;
    }
    return os;
}

//---------------------------------------------------------------------------

SolverQueueItem::SolverQueueItem (SolverQueueItemType type, const ResPool & pool)
    : _type (type)
    , _pool (pool)
{
}


SolverQueueItem::~SolverQueueItem()
{
}

//---------------------------------------------------------------------------

void
SolverQueueItem::copy (const SolverQueueItem *from)
{
}


//---------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////
