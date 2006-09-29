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

  IMPL_PTR_TYPE(Pattern);

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
  bool Pattern::isDefault() const
  { return pimpl().isDefault(); }
  /** */
  bool Pattern::userVisible() const
  { return pimpl().userVisible(); }
  /** */
  std::string Pattern::category() const
  { return pimpl().category().text(); }
  /** */
  Pathname Pattern::icon() const
  { return pimpl().icon(); }
  /** */
  Pathname Pattern::script() const
  { return pimpl().script(); }

  Label Pattern::order() const
  { return pimpl().order(); }

  std::set<std::string> Pattern::install_packages( const Locale & lang ) const
  { return pimpl().install_packages(); }

  const CapSet & Pattern::includes() const
  { return pimpl().includes(); }

  const CapSet & Pattern::extends() const
  { return pimpl().extends(); }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
