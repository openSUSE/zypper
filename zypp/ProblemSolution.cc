
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
#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/SolutionAction.h"
#include "zypp/ProblemSolution.h"
#include "zypp/base/Logger.h"
#include "zypp/solver/detail/Resolver.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////

IMPL_PTR_TYPE(ProblemSolution);

//---------------------------------------------------------------------------

ostream&
operator<<( ostream& os, const ProblemSolution & solution)
{
    os << "Solution:" << endl;
    os << solution._description << endl;
    if ( ! solution._details.empty() )
      os << solution._details << endl;
    os << solution._actions;
    return os;
}

ostream&
operator<<( ostream& os, const ProblemSolutionList & solutionlist)
{
    for (ProblemSolutionList::const_iterator iter = solutionlist.begin(); iter != solutionlist.end(); ++iter) {
	os << *(*iter);
    }
    return os;
}

ostream&
operator<<( ostream& os, const CProblemSolutionList & solutionlist)
{
    for (CProblemSolutionList::const_iterator iter = solutionlist.begin(); iter != solutionlist.end(); ++iter) {
	os << *(*iter) << endl;
    }
    return os;
}

//---------------------------------------------------------------------------

ProblemSolution::ProblemSolution( ResolverProblem_Ptr parent, const string & description, const string & details )
    : _problem (parent)
    , _description (description)
    , _details (details)
{
}


ProblemSolution::~ProblemSolution()
{
}


/**
 * Apply this solution, i.e. execute all of its actions.
 *
 * Returns 'true' on success, 'false' if actions could not be performed.
 **/

bool
ProblemSolution::apply (solver::detail::Resolver & resolver)
{
    DBG << "apply solution " << *this << endl;
    bool ret = true;
    for (solver::detail::CSolutionActionList::const_iterator iter = _actions.begin();
	 iter != _actions.end(); ++iter) {
	solver::detail::SolutionAction_constPtr action = *iter;
	if (! action->execute (resolver))
	{
	    WAR << "apply solution action failed: " << action << endl;
	    ret = false;
	    break;
	}
    }
    return ret;
}


/**
 * Add an action to the actions list.
 **/
void
ProblemSolution::addAction (solver::detail::SolutionAction_constPtr action)
{
    _actions.push_back (action);
}


void
ProblemSolution::clear()
{
    _actions.clear();
}

  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////
