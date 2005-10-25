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

#include "zypp/SolverContext.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Capability::Capability
  //	METHOD TYPE : Ctor
  //
  Capability::Capability( ImplPtr impl_r )
  : _pimpl( impl_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Capability::~Capability
  //	METHOD TYPE : Dtor
  //
  Capability::~Capability()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Capability::sayFriend
  //	METHOD TYPE : capability::constCapabilityImplPtr
  //
  Capability::constImplPtr Capability::sayFriend() const
  { return _pimpl; }

  const ResKind & Capability::refers() const
  { return _pimpl->refers(); }

  std::string Capability::asString() const
  { return _pimpl->asString(); }

  bool Capability::matches( constResolvablePtr resolvable_r,
                            const SolverContext & solverContext_r ) const
  { return _pimpl->matches( resolvable_r, solverContext_r ); }

  bool Capability::matches( constResolvablePtr resolvable_r ) const
  { return _pimpl->matches( resolvable_r, SolverContext() ); }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const Capability & obj )
  {
    return str << *obj.sayFriend();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
