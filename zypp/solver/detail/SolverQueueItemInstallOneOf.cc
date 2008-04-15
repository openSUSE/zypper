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

#include "zypp/base/Logger.h"
#include "zypp/solver/detail/SolverQueueItemInstallOneOf.h"

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
    os << "[" << "InstallOneOf: ";
    for (PoolItemList::const_iterator iter = _oneOfList.begin();
	 iter != _oneOfList.end();
	 iter++)
	os << *iter;
    os << "]";
    
    return os;
}

//---------------------------------------------------------------------------

SolverQueueItemInstallOneOf::SolverQueueItemInstallOneOf (const ResPool & pool, const PoolItemList & itemList)
    : SolverQueueItem (QUEUE_ITEM_TYPE_INSTALL_ONE_OF, pool)
    , _oneOfList (itemList)
{
}


SolverQueueItemInstallOneOf::~SolverQueueItemInstallOneOf()
{
}

//---------------------------------------------------------------------------

bool SolverQueueItemInstallOneOf::addRule (Queue & q, Pool *SATPool)
{
    bool ret = true;
    MIL << "Install one of: " << endl;
    queue_push( &(q), SOLVER_INSTALL_SOLVABLE_ONE_OF );
    for (PoolItemList::const_iterator iter = _oneOfList.begin(); iter != _oneOfList.end(); iter++) {
	Id id = (*iter)->satSolvable().id();
	if (id == ID_NULL) {
	    ERR << *iter << " not found" << endl;
	    ret = false;
	} else {
	    MIL << "    candidate:" << *iter << " with the SAT-Pool ID: " << id << endl;
	    queue_push( &(q), id );    		    
	}
    }
    queue_push( &(q), 0 ); // finish candidate
    
    return ret;
}

SolverQueueItem_Ptr
SolverQueueItemInstallOneOf::copy (void) const
{
    SolverQueueItemInstallOneOf_Ptr new_installOneOf = new SolverQueueItemInstallOneOf (pool(), _oneOfList);
    new_installOneOf->SolverQueueItem::copy(this);
    
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
