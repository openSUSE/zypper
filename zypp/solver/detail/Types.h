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
#define _XDEBUG(x) XXX << x << std::endl;
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

typedef std::list<PoolItem_Ref> PoolItemList;
typedef std::set<PoolItem_Ref> PoolItemSet;
      
DEFINE_PTR_TYPE(Resolver);

DEFINE_PTR_TYPE(ResolverContext);
      
DEFINE_PTR_TYPE(ResolverInfo);
DEFINE_PTR_TYPE(ResolverInfoChildOf);
DEFINE_PTR_TYPE(ResolverInfoConflictsWith);
DEFINE_PTR_TYPE(ResolverInfoContainer);
DEFINE_PTR_TYPE(ResolverInfoDependsOn);
DEFINE_PTR_TYPE(ResolverInfoMisc);
DEFINE_PTR_TYPE(ResolverInfoMissingReq);
DEFINE_PTR_TYPE(ResolverInfoNeededBy);
DEFINE_PTR_TYPE(ResolverInfoObsoletes);

DEFINE_PTR_TYPE(QueueItem);
DEFINE_PTR_TYPE(QueueItemBranch);
DEFINE_PTR_TYPE(QueueItemConflict);
DEFINE_PTR_TYPE(QueueItemEstablish);
DEFINE_PTR_TYPE(QueueItemGroup);
DEFINE_PTR_TYPE(QueueItemInstall);
DEFINE_PTR_TYPE(QueueItemRequire);
DEFINE_PTR_TYPE(QueueItemUninstall);

DEFINE_PTR_TYPE(ResolverQueue);
      
DEFINE_PTR_TYPE(SolutionAction);
typedef std::list<SolutionAction_Ptr> SolutionActionList;
typedef std::list<SolutionAction_constPtr> CSolutionActionList;
DEFINE_PTR_TYPE(TransactionSolutionAction);
DEFINE_PTR_TYPE(InjectSolutionAction);

      
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
