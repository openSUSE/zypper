/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverProblem.cc
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

#include "zypp/solver/detail/ResolverProblem.h"
#include "zypp/solver/detail/ProblemSolution.h"

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

IMPL_PTR_TYPE(ResolverProblem);

//---------------------------------------------------------------------------

string
ResolverProblem::asString ( void ) const
{
    return toString (*this);
}


string
ResolverProblem::toString ( const ResolverProblem & problem )
{
    string ret ("Problem:\n");
    ret += problem._description + "\n";
    ret += problem._details + "\n";
    ret += ProblemSolution::toString (problem._solutions);
    return ret;
}


std::string
ResolverProblem::toString (const ResolverProblemList & problemlist)
{
    string ret;
    for (ResolverProblemList::const_iterator iter = problemlist.begin(); iter != problemlist.end(); ++iter) {
	ret += (*iter)->asString();
    }
    return ret;
}


ostream &
ResolverProblem::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const ResolverProblem & problem)
{
    return os << problem.asString();
}

//---------------------------------------------------------------------------

/**
 * Constructor.
 **/
ResolverProblem::ResolverProblem( const string & description, const string & details )
    : _description (description)
    , _details (details)
{
}

/**
 * Destructor.
 **/
ResolverProblem::~ResolverProblem()
{
}

/**
 * Return the possible solutions to this problem.
 * All problems should have at least 2-3 (mutually exclusive) solutions:
 *
 *	  -  Undo: Do not perform the offending transaction
 *	 (do not install the package that had unsatisfied requirements,
 *	  do not remove	 the package that would break other packages' requirements)
 *
 *	  - Remove referrers: Remove all packages that would break because
 *	they depend on the package that is requested to be removed
 *
 *	  - Ignore: Inject artificial "provides" for a missing requirement
 *	(pretend that requirement is satisfied)
 **/

ProblemSolutionList
ResolverProblem::solutions() const
{
    return _solutions;
}

/**
 * Add a solution to this problem. This class takes over ownership of
 * the problem and will delete it when neccessary.
 **/

void
ResolverProblem::addSolution( ProblemSolution_Ptr solution )
{
    _solutions.push_back (solution);
}

void
ResolverProblem::clear()
{
    _solutions.clear();
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
