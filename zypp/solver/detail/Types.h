/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Types.h
 *
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

#ifndef ZYPP_SOLVER_DETAIL_TYPES_H
#define ZYPP_SOLVER_DETAIL_TYPES_H

#include <iosfwd>
#include <list>
#include <set>
#include <map>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Functional.h"

#include "zypp/PoolItem.h"

#define _DEBUG(x) DBG << x << std::endl;
#define _XDEBUG(x) do { if (base::logger::isExcessive()) XXX << x << std::endl;} while (0)
//#define _DEBUG(x)

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

typedef std::list<PoolItem> PoolItemList;
typedef std::set<PoolItem> PoolItemSet;
      
DEFINE_PTR_TYPE(Resolver);

DEFINE_PTR_TYPE(SolutionAction);
typedef std::list<SolutionAction_Ptr> SolutionActionList;
typedef std::list<SolutionAction_constPtr> CSolutionActionList;
DEFINE_PTR_TYPE(TransactionSolutionAction);
DEFINE_PTR_TYPE(InjectSolutionAction);
DEFINE_PTR_TYPE(SolverQueueItem);
DEFINE_PTR_TYPE(SolverQueueItemUpdate);
DEFINE_PTR_TYPE(SolverQueueItemDelete);
DEFINE_PTR_TYPE(SolverQueueItemInstall);	
DEFINE_PTR_TYPE(SolverQueueItemInstallOneOf);
DEFINE_PTR_TYPE(SolverQueueItemLock);		
      
      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

#endif // ZYPP_SOLVER_DETAIL_TYPES_H
