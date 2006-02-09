/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Pattern.cc
 *
*/
#include <iostream>

#include "zypp/Pattern.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Pattern::Pattern
  //	METHOD TYPE : Ctor
  //
  Pattern::Pattern( const NVRAD & nvrad_r )
  : ResObject( TraitsType::kind, nvrad_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Pattern::~Pattern
  //	METHOD TYPE : Dtor
  //
  Pattern::~Pattern()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	Pattern interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////
  /** */
  std::string patternId() const
  { return pimpl().patternId(); }
  /** */
  bool Pattern::isDefault() const
  { return pimpl().isDefault(); }
  /** */
  bool Pattern::userVisible() const
  { return pimpl().userVisible(); }
  /** */
  std::string Pattern::category() const
  { return pimpl().category(); }
  /** */
  Pathname Pattern::icon() const
  { return pimpl().icon(); }
  /** */
  Pathname Pattern::script() const
  { return pimpl().script(); }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
