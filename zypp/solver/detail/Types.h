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

#include <list>
#include "zypp/base/PtrTypes.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  {
    /////////////////////////////////////////////////////////////////////
    namespace detail
    {
      // A few type names exposed in the public API
      //
      class Resolver;
      typedef Resolver ResolverInternal;	///< Preferred name in API

      class ItemCapKind;
      typedef std::list<ItemCapKind> ItemCapKindList;

      DEFINE_PTR_TYPE(SolverQueueItem);
      typedef std::list<SolverQueueItem_Ptr> SolverQueueItemList;

      DEFINE_PTR_TYPE(SolutionAction);
      typedef std::list<SolutionAction_Ptr> SolutionActionList;

    } // namespace detail
    /////////////////////////////////////////////////////////////////////
  } // namespace solver
  ///////////////////////////////////////////////////////////////////////
} // namespace zypp
/////////////////////////////////////////////////////////////////////////
#endif // ZYPP_SOLVER_DETAIL_TYPES_H
