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
#include "zypp/IdString.h"
#include "zypp/Resolver.h"
#include "zypp/solver/detail/SolverQueueItemDelete.h"

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

IMPL_PTR_TYPE(SolverQueueItemDelete);

//---------------------------------------------------------------------------

std::ostream &
SolverQueueItemDelete::dumpOn( std::ostream & os ) const
{
    os << "[" << (_soft?"Soft":"") << "Delete: "
       << _name << "]";

    return os;
}

//---------------------------------------------------------------------------

SolverQueueItemDelete::SolverQueueItemDelete (const ResPool & pool, std::string name, bool soft)
    : SolverQueueItem (QUEUE_ITEM_TYPE_DELETE, pool)
    , _name (name)
    , _soft (soft)
{
}


SolverQueueItemDelete::~SolverQueueItemDelete()
{
}

//---------------------------------------------------------------------------

bool SolverQueueItemDelete::addRule (_Queue & q)
{
#define MAYBE_CLEANDEPS (pool().resolver().cleandepsOnRemove()?SOLVER_CLEANDEPS:0)

    ::Id id = IdString(_name).id();
    if (_soft) {
	queue_push( &(q), SOLVER_ERASE | SOLVER_SOLVABLE_NAME | SOLVER_WEAK | MAYBE_CLEANDEPS );
    } else {
	queue_push( &(q), SOLVER_ERASE | SOLVER_SOLVABLE_NAME | MAYBE_CLEANDEPS );
    }
    queue_push( &(q), id);

    MIL << "Delete " << _name << (_soft ? "(soft)" : "")
	<< " with SAT-Pool: " << id << endl;
    return true;
}

SolverQueueItem_Ptr
SolverQueueItemDelete::copy (void) const
{
    SolverQueueItemDelete_Ptr new_delete = new SolverQueueItemDelete (pool(), _name);
    new_delete->SolverQueueItem::copy(this);

    new_delete->_soft = _soft;
    return new_delete;
}

int
SolverQueueItemDelete::cmp (SolverQueueItem_constPtr item) const
{
    int cmp = this->compare (item);
    if (cmp != 0)
        return cmp;
    SolverQueueItemDelete_constPtr del = dynamic_pointer_cast<const SolverQueueItemDelete>(item);
    if (_name != del->_name) {
	return _name.compare(del->_name);
    }
    return 0;
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
