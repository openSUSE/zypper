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
#include "zypp/ResObject.h"
#include "zypp/PoolItem.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(Resolvable);

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Resolvable::Resolvable
  //	METHOD TYPE : Ctor
  //
  Resolvable::Resolvable( const sat::Solvable & solvable_r )
  : sat::Solvable( solvable_r )
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
  //	METHOD NAME : Resolvable::poolItem
  //	METHOD TYPE : PoolItem
  //
  PoolItem Resolvable::poolItem() const
  { return PoolItem( *this ); }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Resolvable::dumpOn
  //	METHOD TYPE : std::ostream &
  //
  std::ostream & Resolvable::dumpOn( std::ostream & str ) const
  { return str << satSolvable(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
