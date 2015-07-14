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
#include "zypp/solver/detail/SolverQueueItemInstallOneOf.h"
#include "zypp/sat/Pool.h"

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

IMPL_PTR_TYPE(SolverQueueItemInstallOneOf);

//---------------------------------------------------------------------------

std::ostream &
SolverQueueItemInstallOneOf::dumpOn( std::ostream & os ) const
{
    os << "[" << (_soft?"Soft":"") << "InstallOneOf: ";
    for (PoolItemList::const_iterator iter = _oneOfList.begin();
	 iter != _oneOfList.end();
	 iter++)
	os << *iter;
    os << "]";

    return os;
}

//---------------------------------------------------------------------------

SolverQueueItemInstallOneOf::SolverQueueItemInstallOneOf (const ResPool & pool, const PoolItemList & itemList,
							  bool soft)
    : SolverQueueItem (QUEUE_ITEM_TYPE_INSTALL_ONE_OF, pool)
    , _oneOfList (itemList)
    , _soft (soft)
{
}


SolverQueueItemInstallOneOf::~SolverQueueItemInstallOneOf()
{
}

//---------------------------------------------------------------------------

bool SolverQueueItemInstallOneOf::addRule (_Queue & q)
{
    bool ret = true;
    MIL << "Install one of " << (_soft ? "(soft):" : ":")<< endl;
    Queue qs;

    if (_soft) {
	queue_push( &(q), SOLVER_INSTALL | SOLVER_SOLVABLE_ONE_OF | SOLVER_WEAK);
    } else {
	queue_push( &(q), SOLVER_INSTALL | SOLVER_SOLVABLE_ONE_OF );
    }

    queue_init(&qs);
    for (PoolItemList::const_iterator iter = _oneOfList.begin(); iter != _oneOfList.end(); iter++) {
	Id id = (*iter)->satSolvable().id();
	if (id == ID_NULL) {
	    ERR << *iter << " not found" << endl;
	    ret = false;
	} else {
	    MIL << "    candidate:" << *iter << " with the SAT-Pool ID: " << id << endl;
	    queue_push( &(qs), id );
	}
    }
    sat::Pool satPool( sat::Pool::instance() );
    queue_push( &(q), pool_queuetowhatprovides(satPool.get(), &qs));
    queue_free(&qs);

    return ret;
}

SolverQueueItem_Ptr
SolverQueueItemInstallOneOf::copy (void) const
{
    SolverQueueItemInstallOneOf_Ptr new_installOneOf = new SolverQueueItemInstallOneOf (pool(), _oneOfList);
    new_installOneOf->SolverQueueItem::copy(this);
    new_installOneOf->_soft = _soft;

    return new_installOneOf;
}

int
SolverQueueItemInstallOneOf::cmp (SolverQueueItem_constPtr item) const
{
    int cmp = this->compare (item);
    if (cmp != 0)
        return cmp;
    SolverQueueItemInstallOneOf_constPtr install = dynamic_pointer_cast<const SolverQueueItemInstallOneOf>(item);

    return (_oneOfList == install->_oneOfList) ? 0 : -1; // more evaluation would be not useful
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
