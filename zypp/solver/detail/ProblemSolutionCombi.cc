
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

#include <sstream>

#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

#include "zypp/solver/detail/ProblemSolutionCombi.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

IMPL_PTR_TYPE(ProblemSolutionCombi);

//---------------------------------------------------------------------------

ProblemSolutionCombi::ProblemSolutionCombi( ResolverProblem_Ptr parent)
    : ProblemSolution (parent, "", "")
      , actNumber(0)
{
    _description = "";
    _details = "";
}

void ProblemSolutionCombi::addSingleAction( Capability capability, const TransactionKind action)
{
    addAction (new TransactionSolutionAction(capability, action));
    actNumber++;
}

void ProblemSolutionCombi::addSingleAction( PoolItem item, const TransactionKind action)
{
    addAction (new TransactionSolutionAction(item, action));
    actNumber++;
}

void ProblemSolutionCombi::addSingleAction( SolverQueueItem_Ptr item, const TransactionKind action)
{
    addAction (new TransactionSolutionAction(item, action));
    actNumber++;
}

void ProblemSolutionCombi::addDescription( const std::string description)
{
    if ( _description.size() == 0
	 && _details.size() == 0) {
	 // first entry
	_description = description;
    } else {
	if ( _description.size() > 0
	     && _details.size() == 0) {
	    // second entry
	    _details = _description;
	    _description = _("Following actions will be done:");
	}
	// all other
	_details += "\n";
	_details += description;
    }
}

void ProblemSolutionCombi::addFrontDescription( const std::string & description )
{
    if ( _description.size() == 0
	 && _details.size() == 0) {
	 // first entry
	_description = description;
    } else {
	if ( _description.size() > 0
	     && _details.size() == 0) {
	    // second entry
	    _details = _description;
	    _description = _("Following actions will be done:");
	}
	// all other
        std::string tmp( _details );
	_details = description;
	_details += "\n";
        _details += tmp;
    }
}

      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////
