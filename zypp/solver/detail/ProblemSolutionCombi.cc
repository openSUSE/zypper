
/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ProblemSolution.cc
 *
 * Easy-to use interface to the ZYPP dependency resolver
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
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
#define ZYPP_USE_RESOLVER_INTERNALS

#include "zypp/solver/detail/ProblemSolutionCombi.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  {
    /////////////////////////////////////////////////////////////////////
    namespace detail
    {
      ProblemSolutionCombi::ProblemSolutionCombi()
      {}

      void ProblemSolutionCombi::addSingleAction( Capability capability, TransactionKind action)
      { addAction( new TransactionSolutionAction( capability, action ) ); }

      void ProblemSolutionCombi::addSingleAction( PoolItem item, TransactionKind action )
      { addAction( new TransactionSolutionAction( item, action ) ); }

      void ProblemSolutionCombi::addSingleAction( SolverQueueItem_Ptr item, TransactionKind action )
      { addAction( new TransactionSolutionAction( item, action ) ); }

    } // namespace detail
    /////////////////////////////////////////////////////////////////////
  } // namespace solver
  ///////////////////////////////////////////////////////////////////////
} // namespace zypp
/////////////////////////////////////////////////////////////////////////
