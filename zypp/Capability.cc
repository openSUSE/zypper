/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Capability.cc
 *
*/
#include <iostream>

#include "zypp/Capability.h"
#include "zypp/capability/CapabilityImpl.h"
#include "zypp/capability/NullCap.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  const Capability Capability::noCap;

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Capability::Capability
  //	METHOD TYPE : Ctor
  //
  Capability::Capability()
  : _pimpl( capability::NullCap::instance() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Capability::Capability
  //	METHOD TYPE : Ctor
  //
  Capability::Capability( Impl_Ptr impl_r )
  : _pimpl( impl_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Capability::~Capability
  //	METHOD TYPE : Dtor
  //
  Capability::~Capability()
  {}

  const Resolvable::Kind & Capability::refers() const
  { return _pimpl->refers(); }

  bool Capability::relevant() const
  { return _pimpl->relevant(); }

  CapMatch Capability::matches( const Capability & rhs ) const
  { return _pimpl->matches( rhs._pimpl.getPtr() ); }

  std::string Capability::asString() const
  { return _pimpl->asString(); }

  std::string Capability::index() const
  { return _pimpl->index(); }

  Rel Capability::op() const
  { return _pimpl->op(); }

  Edition Capability::edition() const
  { return _pimpl->edition(); }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const Capability & obj )
  {
    return str << obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
