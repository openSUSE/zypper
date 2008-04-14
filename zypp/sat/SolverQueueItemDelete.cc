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
#include "zypp/sat/SolverQueueItemDelete.h"

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
    os << "[" << (_soft?"Soft":"") << "Delete: ";
    os << _name;

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

SolverQueueItem_Ptr
SolverQueueItemDelete::copy (void) const
{
    SolverQueueItemDelete_Ptr new_delete = new SolverQueueItemDelete (pool(), _name);
    new_delete->SolverQueueItem::copy(this);

    new_delete->_soft = _soft;
    return new_delete;
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
