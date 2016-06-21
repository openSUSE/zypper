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

#include "zypp/base/LogTools.h"

#include "zypp/ResolverProblem.h"
#include "zypp/ProblemSolution.h"

using std::endl;

/////////////////////////////////////////////////////////////////////////
namespace zypp
{
  IMPL_PTR_TYPE(ResolverProblem);

  /////////////////////////////////////////////////////////////////////////
  namespace
  {
    // HACK for bsc#985674: filter duplicate solutions
    //
    inline bool solutionInList( const ProblemSolutionList & solutions_r, const ProblemSolution_Ptr & solution_r )
    {
      for ( const ProblemSolution_Ptr & solution : solutions_r )
      {
	if ( solution->description()	== solution_r->description()
	  && solution->details()	== solution_r->details()
	  && solution->actions().size()	== solution_r->actions().size() )
	  return true;
      }
      return false;
    }
  } // namespace
  /////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  /// \class ResolverProblem::Impl
  /// \brief ResolverProblem implementation.
  ///////////////////////////////////////////////////////////////////
  struct ResolverProblem::Impl
  {
    Impl()
    {}

    Impl( std::string && description )
    : _description( std::move(description) )
    {}

    Impl( std::string && description, std::string && details )
    : _description( std::move(description) )
    , _details( std::move(details) )
    {}

    std::string		_description;
    std::string		_details;
    ProblemSolutionList	_solutions;

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  ResolverProblem::ResolverProblem()
  : _pimpl( new Impl() )
  {}

  ResolverProblem::ResolverProblem( std::string description )
  : _pimpl( new Impl( std::move(description) ) )
  {}

  ResolverProblem::ResolverProblem( std::string description, std::string details )
  : _pimpl( new Impl( std::move(description), std::move(details) ) )
  {}

  ResolverProblem::~ResolverProblem()
  {}


  const std::string & ResolverProblem::description() const
  { return _pimpl->_description; }

  const std::string & ResolverProblem::details() const
  { return _pimpl->_details; }

  const ProblemSolutionList & ResolverProblem::solutions() const
  { return _pimpl->_solutions; }


  void ResolverProblem::setDescription( std::string description )
  { _pimpl->_description = std::move(description); }

  void ResolverProblem::setDetails( std::string details )
  { _pimpl->_details = std::move(details); }

  void ResolverProblem::addSolution( ProblemSolution_Ptr solution, bool inFront )
  {
    if ( ! solutionInList( _pimpl->_solutions, solution ) )	// bsc#985674: filter duplicate solutions
    {
      if (inFront)
      { _pimpl->_solutions.push_front( solution ); }
      else
      { _pimpl->_solutions.push_back( solution ); }
    }
  }


  std::ostream & operator<<( std::ostream & os, const ResolverProblem & obj )
  {
    os << "Problem:" << endl;
    os << "==============================" << endl;
    os << obj.description() << endl;
    os << obj.details() << endl;
    os << "------------------------------" << endl;
    os << obj.solutions();
    os << "==============================" << endl;
    return os;
  }

  std::ostream & operator<<( std::ostream & os, const ResolverProblemList & obj )
  { return dumpRange( os, obj.begin(), obj.end(), "", "", ", ", "", "" ); }

} // namespace zypp
/////////////////////////////////////////////////////////////////////////
