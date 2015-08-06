/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResObject.cc
 *
*/

#include "zypp/ResObject.h"

using namespace zypp;
using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{

  IMPL_PTR_TYPE(ResObject);

  ResObject::ResObject( const sat::Solvable & solvable_r )
  : Resolvable( solvable_r )
  {}

  ResObject::~ResObject()
  {}

  std::ostream & ResObject::dumpOn( std::ostream & str ) const
  {
    return Resolvable::dumpOn( str );
  }

} // namespace zypp
///////////////////////////////////////////////////////////////////

#include "zypp/ResObjects.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ResObject::Ptr makeResObject( const sat::Solvable & solvable_r )
  {
    if ( ! solvable_r )
      return 0;

    ResKind kind( solvable_r.kind() );
#define OUTS(X)  if ( kind == ResTraits<X>::kind ) return make<X>( solvable_r );
    OUTS( Package );
    OUTS( Patch );
    OUTS( Pattern );
    OUTS( Product );
    OUTS( SrcPackage );
    OUTS( Application );
#undef OUTS
    // unknow => return a plain ResObject
    return new ResObject( solvable_r );
  }
} // namespace zypp
///////////////////////////////////////////////////////////////////
