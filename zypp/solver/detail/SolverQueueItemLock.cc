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
#include "zypp/solver/detail/SolverQueueItemLock.h"

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

IMPL_PTR_TYPE(SolverQueueItemLock);

//---------------------------------------------------------------------------

std::ostream &
SolverQueueItemLock::dumpOn( std::ostream & os ) const
{
    os << "[" << (_soft?"Soft":"") << "Lock: " <<
	_item << "]";

    return os;
}

//---------------------------------------------------------------------------

SolverQueueItemLock::SolverQueueItemLock (const ResPool & pool,
					      const PoolItem & item, bool soft)
    : SolverQueueItem (QUEUE_ITEM_TYPE_LOCK, pool)
    , _item (item)
    , _soft (soft)
{
}


SolverQueueItemLock::~SolverQueueItemLock()
{
}

//---------------------------------------------------------------------------

bool SolverQueueItemLock::addRule (_Queue & q)
{
    ::Id id = _item.satSolvable().id();
    if (id == ID_NULL) {
	ERR << "Lock : " << _item << " not found" << endl;
	return false;
    }
    MIL << "Lock " << _item << " with the SAT-Pool ID: " << id << endl;
    if (_item.status().isInstalled()) {
	if (_soft) {
	    queue_push( &(q), SOLVER_INSTALL | SOLVER_SOLVABLE | SOLVER_WEAK );
	} else {
	    queue_push( &(q), SOLVER_INSTALL | SOLVER_SOLVABLE );
	}
    } else {
	if (_soft) {
	    queue_push( &(q), SOLVER_ERASE | SOLVER_SOLVABLE | SOLVER_WEAK );
	} else {
	    queue_push( &(q), SOLVER_ERASE | SOLVER_SOLVABLE );
	}
    }
    queue_push( &(q), id );
    return true;
}

SolverQueueItem_Ptr
SolverQueueItemLock::copy (void) const
{
    SolverQueueItemLock_Ptr new_lock = new SolverQueueItemLock (pool(), _item);
    new_lock->SolverQueueItem::copy(this);

    new_lock->_soft = _soft;
    return new_lock;
}

int
SolverQueueItemLock::cmp (SolverQueueItem_constPtr item) const
{
    int cmp = this->compare (item);
    if (cmp != 0)
        return cmp;
    SolverQueueItemLock_constPtr lock = dynamic_pointer_cast<const SolverQueueItemLock>(item);
    return compareByNVRA (_item, lock->_item);
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
