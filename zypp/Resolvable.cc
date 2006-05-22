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

#include "zypp/ResObject.h"
#include "zypp/Source.h"

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
  //	Resolvable interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  std::ostream & Resolvable::dumpOn( std::ostream & str ) const
  { return _pimpl->dumpOn( str ); }

  const Resolvable::Kind & Resolvable::kind() const
  { return _pimpl->kind(); }

  const std::string & Resolvable::name() const
  { return _pimpl->name(); }

  const Edition & Resolvable::edition() const
  { return _pimpl->edition(); }

  const Arch & Resolvable::arch() const
  { return _pimpl->arch(); }

  const CapSet & Resolvable::dep( Dep which_r ) const
  { return _pimpl->deps()[which_r]; }

  const Dependencies & Resolvable::deps() const
  { return _pimpl->deps(); }


  void Resolvable::injectProvides( const Capability & cap_r )
  { return _pimpl->injectProvides( cap_r ); }

  void Resolvable::injectRequires( const Capability & cap_r )
  { return _pimpl->injectRequires( cap_r ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
