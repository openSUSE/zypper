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
#include "zypp/IdStringType.h"
#include "zypp/solver/detail/SolverQueueItemInstall.h"

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

IMPL_PTR_TYPE(SolverQueueItemInstall);

//---------------------------------------------------------------------------

std::ostream &
SolverQueueItemInstall::dumpOn( std::ostream & os ) const
{
    os << "[" << (_soft?"Soft":"") << "Install: "
    << _name
    << "]";

    return os;
}

//---------------------------------------------------------------------------

SolverQueueItemInstall::SolverQueueItemInstall (const ResPool & pool, std::string name, bool soft)
    : SolverQueueItem (QUEUE_ITEM_TYPE_INSTALL, pool)
    , _name (name)
    , _soft (soft)
{
}


SolverQueueItemInstall::~SolverQueueItemInstall()
{
}

//---------------------------------------------------------------------------

bool SolverQueueItemInstall::addRule (sat::detail::CQueue & q)
{
    ::Id id = IdString(_name).id();
    if (_soft) {
	queue_push( &(q), SOLVER_INSTALL | SOLVER_SOLVABLE_NAME | SOLVER_WEAK  );
    } else {
	queue_push( &(q), SOLVER_INSTALL | SOLVER_SOLVABLE_NAME );
    }
    queue_push( &(q), id);

    MIL << "Install " << _name << (_soft ? "(soft)" : "")
	<< " with SAT-PoolID: " << id << endl;
    return true;
}

SolverQueueItem_Ptr
SolverQueueItemInstall::copy (void) const
{
    SolverQueueItemInstall_Ptr new_install = new SolverQueueItemInstall (pool(), _name);
    new_install->SolverQueueItem::copy(this);

    new_install->_soft = _soft;
    return new_install;
}

int
SolverQueueItemInstall::cmp (SolverQueueItem_constPtr item) const
{
    int cmp = this->compare (item);
    if (cmp != 0)
        return cmp;
    SolverQueueItemInstall_constPtr ins = dynamic_pointer_cast<const SolverQueueItemInstall>(item);
    if (_name != ins->_name) {
	return _name.compare(ins->_name);
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
