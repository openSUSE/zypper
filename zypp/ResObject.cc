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

#include <zypp/base/Logger.h>
#include "zypp/ResObject.h"
#include "zypp/Repository.h"
#include "zypp/sat/SolvAttr.h"
#include "zypp/sat/Solvable.h"
#include "zypp/sat/Repo.h"

using namespace zypp;
using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(ResObject);

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResObject::ResObject
  //	METHOD TYPE : Ctor
  //
  ResObject::ResObject( const sat::Solvable & solvable_r )
  : Resolvable( solvable_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResObject::~ResObject
  //	METHOD TYPE : Dtor
  //
  ResObject::~ResObject()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResObject::dumpOn
  //	METHOD TYPE : std::ostream &
  //
  std::ostream & ResObject::dumpOn( std::ostream & str ) const
  {
    return Resolvable::dumpOn( str );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	ResObject interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  Text ResObject::summary() const
  {
    return lookupStrAttribute( sat::SolvAttr::summary );
  }

  Text ResObject::description() const
  { return lookupStrAttribute( sat::SolvAttr::description ); }

  Text ResObject::insnotify() const
  { return lookupStrAttribute( sat::SolvAttr::insnotify ); }

  Text ResObject::delnotify() const
  { return lookupStrAttribute( sat::SolvAttr::delnotify ); }

  License ResObject::licenseToConfirm() const
  { return lookupStrAttribute( sat::SolvAttr::eula ); }

  ByteCount ResObject::size() const
  { return ByteCount( lookupNumAttribute( sat::SolvAttr::size ), ByteCount::K ); }

  ByteCount ResObject::downloadSize() const
  { return ByteCount( lookupNumAttribute( sat::SolvAttr::downloadsize ), ByteCount::K ); }

  RepoInfo ResObject::repoInfo() const
  { return repo().info(); }

  unsigned ResObject::mediaNr() const
  { return lookupNumAttribute( sat::SolvAttr::medianr ); }

  bool ResObject::installOnly() const
  { return false; }

  Date ResObject::buildtime() const
  { return Date(); }

  Date ResObject::installtime() const
  { return Date(); }

  const DiskUsage & ResObject::diskusage() const
  {
    static DiskUsage _du;
    return _du;
  }

   /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#include "zypp/ResObjects.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ResObject::Ptr makeResObject( const sat::Solvable & solvable_r )
  {
    ResKind kind( solvable_r.kind() );
#define OUTS(X)  if ( kind == ResTraits<X>::kind ) return make<X>( solvable_r );
    OUTS( Atom );
    OUTS( Message );
    OUTS( Package );
    OUTS( Patch );
    OUTS( Pattern );
    OUTS( Product );
    OUTS( Script );
    OUTS( SrcPackage );
#undef OUTS
    return 0;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
