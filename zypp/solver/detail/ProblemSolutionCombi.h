/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Resolver_problems.cc
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

#ifndef ZYPP_SOLVER_DETAIL_PROBLEMSOLUTIONINSTALL_H
#define ZYPP_SOLVER_DETAIL_PROBLEMSOLUTIONINSTALL_H
#ifndef ZYPP_USE_RESOLVER_INTERNALS
#error Do not directly include this file!
#else

#include <string>

#include "zypp/ProblemSolution.h"
#include "zypp/solver/detail/SolutionAction.h" // TransactionKind

/////////////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  {
    /////////////////////////////////////////////////////////////////////
    namespace detail
    {
      /////////////////////////////////////////////////////////////////////////
      /// \class ProblemSolutionCombi
      /// \brief Class representing one possible solution to one problem found during resolving.
      ///
      /// This problem solution is a combination of different actions,
      /// e.G. install, delete, keep different resolvables.
      /////////////////////////////////////////////////////////////////////////
      class ProblemSolutionCombi : public ProblemSolution
      {
      public:
	/** Constructor. */
	ProblemSolutionCombi();

	/**
	 * Add a single action of an item
	 */
	void addSingleAction( PoolItem item, TransactionKind action );

	/**
	 * Add a single action of a capability
	 */
	void addSingleAction( Capability capability, TransactionKind action );

	/**
	 * Add a single action of a SolverQueueItem
	 */
	void addSingleAction( SolverQueueItem_Ptr item, TransactionKind action );

	/**
	 * returns the number of actions
	 */
	size_t actionCount()
	{ return actions().size(); }

	/**
	 * Set description text (append)
	 */
	void addDescription( std::string description )
	{ pushDescriptionDetail( std::move(description) ); }

	/**
	 * Set description text (prepend)
	 */
	void addFrontDescription( std::string description )
	{ pushDescriptionDetail( std::move(description), /*front*/true ); }
      };

    } // namespace detail
    /////////////////////////////////////////////////////////////////////
  } // namespace solver
  ///////////////////////////////////////////////////////////////////////
} // namespace zypp
/////////////////////////////////////////////////////////////////////////
#endif // ZYPP_USE_RESOLVER_INTERNALS
#endif // ZYPP_SOLVER_DETAIL_PROBLEMSOLUTIONAINSTALL_H

