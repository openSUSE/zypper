/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Resolvable.cc
 *
*/
#include "zypp/Resolvable.h"
#include "zypp/detail/ResolvableImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  IMPL_PTR_TYPE(Resolvable)

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Resolvable::Resolvable
  //	METHOD TYPE : Ctor
  //
  Resolvable::Resolvable( detail::ResolvableImplPtr impl_r )
  : _pimpl( impl_r )
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

  const std::string & Resolvable::name() const
  { return _pimpl->name(); }

  const Edition & Resolvable::edition() const
  { return _pimpl->edition(); }

  const Arch & Resolvable::arch() const
  { return _pimpl->arch(); }

  const Dependencies & Resolvable::deps() const
  { return _pimpl->deps(); }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Resolvable::~Resolvable
  //	METHOD TYPE : Dtor
  //
  detail::constResolvableImplPtr Resolvable::sayFriend() const
  { return _pimpl; }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const Resolvable & obj )
  {
    return str << *obj.sayFriend();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
