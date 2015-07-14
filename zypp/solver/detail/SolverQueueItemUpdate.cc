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
#include "zypp/solver/detail/SolverQueueItemUpdate.h"

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

IMPL_PTR_TYPE(SolverQueueItemUpdate);

//---------------------------------------------------------------------------

std::ostream &
SolverQueueItemUpdate::dumpOn( std::ostream & os ) const
{
    os << "[" << (_soft?"Soft":"") << "Update: " <<
	_item << "]";

    return os;
}

//---------------------------------------------------------------------------

SolverQueueItemUpdate::SolverQueueItemUpdate (const ResPool & pool,
					      const PoolItem & item, bool soft)
    : SolverQueueItem (QUEUE_ITEM_TYPE_UPDATE, pool)
    , _item (item)
    , _soft (soft)
{
}


SolverQueueItemUpdate::~SolverQueueItemUpdate()
{
}

//---------------------------------------------------------------------------

bool SolverQueueItemUpdate::addRule (_Queue & q)
{
    ::Id id = _item.satSolvable().id();
    if (id == ID_NULL) {
	ERR << "Update explicit: " << _item << " not found" << endl;
	return false;
    }
    MIL << "Update explicit " << _item << " with the SAT-Pool ID: " << id << endl;
    queue_push( &(q), SOLVER_UPDATE | SOLVER_SOLVABLE );
    queue_push( &(q), id );
    return true;
}

SolverQueueItem_Ptr
SolverQueueItemUpdate::copy (void) const
{
    SolverQueueItemUpdate_Ptr new_update = new SolverQueueItemUpdate (pool(), _item);
    new_update->SolverQueueItem::copy(this);

    new_update->_soft = _soft;
    return new_update;
}

int
SolverQueueItemUpdate::cmp (SolverQueueItem_constPtr item) const
{
    int cmp = this->compare (item);
    if (cmp != 0)
        return cmp;
    SolverQueueItemUpdate_constPtr update = dynamic_pointer_cast<const SolverQueueItemUpdate>(item);
    return compareByNVRA (_item.resolvable(), update->_item.resolvable());
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
