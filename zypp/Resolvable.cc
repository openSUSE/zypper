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
#include <iostream>

#include "zypp/Resolvable.h"
#include "zypp/detail/ResolvableImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Resolvable::Resolvable
  //	METHOD TYPE : Ctor
  //
  Resolvable::Resolvable( const Kind & kind_r,
                          const std::string & name_r,
                          const Edition & edition_r,
                          const Arch & arch_r )
  : _pimpl( new Impl( kind_r, name_r, edition_r, arch_r ) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Resolvable::~Resolvable
  //	METHOD TYPE : Dtor
  //
  Resolvable::~Resolvable()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Resolvable::dumpOn
  //	METHOD TYPE : std::ostream
  //
  std::ostream & Resolvable::dumpOn( std::ostream & str ) const
  {
    return str << '[' << kind() << ']'
    << name() << '-' << edition() << '.' << arch();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	Resolvable interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  const Resolvable::Kind & Resolvable::kind() const
  { return _pimpl->kind(); }

  const std::string & Resolvable::name() const
  { return _pimpl->name(); }

  const Edition & Resolvable::edition() const
  { return _pimpl->edition(); }

  const Arch & Resolvable::arch() const
  { return _pimpl->arch(); }

  const Dependencies & Resolvable::deps() const
  { return _pimpl->deps(); }

  void Resolvable::setDeps( const Dependencies & val_r )
  { _pimpl->setDeps( val_r ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
