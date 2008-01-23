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

  std::ostream & Resolvable::dumpOn( std::ostream & str ) const
  {
    //::dumpOn( str, *(sat::Solvable*)(this) );
    //return str << sat::Solvable::dumpOn(str);
    return str;
  }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
