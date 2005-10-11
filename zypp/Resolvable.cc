/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 \file	zypp/Resolvable.cc

 \brief	.

*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/Resolvable.h"
#include "zypp/detailResolvableImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Resolvable::Resolvable
  //	METHOD TYPE : Ctor
  //
  Resolvable::Resolvable()
  : _pimpl( new ResolvableImpl )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Resolvable::~Resolvable
  //	METHOD TYPE : Dtor
  //
  Resolvable::~Resolvable()
  {}

  const ResKind & Resolvable::kind() const
  { return _pimpl->kind(); }
  const ResName & Resolvable::name() const
  { return _pimpl->name(); }
  const ResEdition & Resolvable::edition() const
  { return _pimpl->edition(); }
  const ResArch & Resolvable::arch() const
  { return _pimpl->arch(); }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const Resolvable & obj )
  {
    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
