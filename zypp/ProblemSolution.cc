
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

#include "zypp/base/Gettext.h"
#include "zypp/solver/detail/SolutionAction.h"
#include "zypp/ProblemSolution.h"
#include "zypp/base/Logger.h"
#include "zypp/solver/detail/Resolver.h"

using std::endl;

/////////////////////////////////////////////////////////////////////////
namespace zypp
{
  IMPL_PTR_TYPE(ProblemSolution);

  ///////////////////////////////////////////////////////////////////
  /// \class ProblemSolution::Impl
  /// \brief ProblemSolution implementation.
  ///////////////////////////////////////////////////////////////////
  struct ProblemSolution::Impl
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
    SolutionActionList	_actions;

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  ProblemSolution::ProblemSolution()
  : _pimpl( new Impl() )
  {}

  ProblemSolution::ProblemSolution( std::string description )
  : _pimpl( new Impl( std::move(description) ) )
  {}

  ProblemSolution::ProblemSolution( std::string description, std::string details )
  : _pimpl( new Impl( std::move(description), std::move(details) ) )
  {}

  ProblemSolution::~ProblemSolution()
  {}


  const std::string & ProblemSolution::description() const
  { return _pimpl->_description; }

  const std::string & ProblemSolution::details() const
  { return _pimpl->_details; }

  const ProblemSolution::SolutionActionList & ProblemSolution::actions() const
  { return _pimpl->_actions; }


  void ProblemSolution::setDescription( std::string description )
  { _pimpl->_description = std::move(description); }

  void ProblemSolution::setDetails( std::string details )
  { _pimpl->_details += "\n"; _pimpl->_details += std::move(details); }

  void ProblemSolution::pushDescriptionDetail( std::string description, bool front )
  {
    if ( _pimpl->_details.empty() )
    {
      if ( _pimpl->_description.empty() )	// first entry
      {
	_pimpl->_description = std::move(description);
	return;
      }
      else					// second entry: form headline in _description
      {
	_pimpl->_description.swap( _pimpl->_details );
	_pimpl->_description = _("Following actions will be done:");
      }
    }
    if ( front )
    { _pimpl->_details.swap( description ); }
    _pimpl->_details += "\n";
    _pimpl->_details += std::move(description);
  }

  void ProblemSolution::addAction( solver::detail::SolutionAction_Ptr action )
  { _pimpl->_actions.push_back( action ); }



  std::ostream & operator<<( std::ostream & os, const ProblemSolution & obj )
  {
    os << "Solution:" << endl;
    os << obj.description() << endl;
    if ( ! obj.details().empty() )
      os << obj.details() << endl;
    os << obj.actions();
    return os;
  }

  std::ostream & operator<<( std::ostream & os, const ProblemSolutionList & obj )
  {
    for ( const auto & ptr: obj )
    { os << ptr; }
    return os;
  }

} // namespace zypp
/////////////////////////////////////////////////////////////////////////
