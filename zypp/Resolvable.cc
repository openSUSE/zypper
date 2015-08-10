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
{
  IMPL_PTR_TYPE(Resolvable);

  Resolvable::Resolvable( const sat::Solvable & solvable_r )
  : _solvable( solvable_r )
  {}

  Resolvable::~Resolvable()
  {}

  PoolItem Resolvable::poolItem() const
  { return PoolItem( *this ); }

  std::ostream & Resolvable::dumpOn( std::ostream & str ) const
  { return str << satSolvable(); }

} // namespace zypp
///////////////////////////////////////////////////////////////////
