/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Selection.cc
 *
*/
#include <iostream>

#include "zypp/Selection.h"
#include "zypp/detail/SelectionImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Selection::Selection
  //	METHOD TYPE : Ctor
  //
  Selection::Selection( detail::SelectionImplPtr impl_r )
  :  Resolvable( impl_r )
  , _pimpl( impl_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Selection::~Selection
  //	METHOD TYPE : Dtor
  //
  Selection::~Selection()
  {}

  std::string Selection::summary() const
  { return _pimpl->summary(); }

  std::list<std::string> Selection::description() const
  { return _pimpl->description(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
