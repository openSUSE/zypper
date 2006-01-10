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
                          const NVRAD & nvrad_r )
  : _pimpl( new Impl( kind_r, nvrad_r ) )
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

  const CapSet & Resolvable::provides() const
  { return _pimpl->deps().provides; }

  const CapSet & Resolvable::prerequires() const
  { return _pimpl->deps().prerequires; }

  const CapSet & Resolvable::requires() const
  { return _pimpl->deps().requires; }

  const CapSet & Resolvable::conflicts() const
  { return _pimpl->deps().conflicts; }

  const CapSet & Resolvable::obsoletes() const
  { return _pimpl->deps().obsoletes; }

  const CapSet & Resolvable::recommends() const
  { return _pimpl->deps().recommends; }

  const CapSet & Resolvable::suggests() const
  { return _pimpl->deps().suggests; }

  const CapSet & Resolvable::freshens() const
  { return _pimpl->deps().freshens; }

  const Dependencies & Resolvable::deps() const
  { return _pimpl->deps(); }


  void Resolvable::injectProvides( const Capability & cap_r )
  { return _pimpl->injectProvides( cap_r ); }

  void Resolvable::injectRequires( const Capability & cap_r )
  { return _pimpl->injectRequires( cap_r ); }


  void Resolvable::deprecatedSetDeps( const Dependencies & val_r )
  { _pimpl->deprecatedSetDeps( val_r ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
