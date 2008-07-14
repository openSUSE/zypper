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
#include "zypp/sat/SolvAttr.h"
#include "zypp/sat/Solvable.h"
#include "zypp/Repository.h"
#include "zypp/RepoInfo.h"

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

  std::string ResObject::summary( const Locale & lang_r ) const
  { return lookupStrAttribute( sat::SolvAttr::summary, lang_r ); }

  std::string ResObject::description( const Locale & lang_r ) const
  { return lookupStrAttribute( sat::SolvAttr::description, lang_r ); }

  std::string ResObject::insnotify( const Locale & lang_r ) const
  { return lookupStrAttribute( sat::SolvAttr::insnotify, lang_r ); }

  std::string ResObject::delnotify( const Locale & lang_r ) const
  { return lookupStrAttribute( sat::SolvAttr::delnotify, lang_r ); }

  std::string ResObject::licenseToConfirm( const Locale & lang_r ) const
  { return lookupStrAttribute( sat::SolvAttr::eula, lang_r ); }

  ByteCount ResObject::installSize() const
  { return ByteCount( lookupNumAttribute( sat::SolvAttr::installsize ), ByteCount::K ); }

  ByteCount ResObject::downloadSize() const
  { return ByteCount( lookupNumAttribute( sat::SolvAttr::downloadsize ), ByteCount::K ); }

  unsigned ResObject::mediaNr() const
  { return lookupNumAttribute( sat::SolvAttr::medianr ); }

#warning DUMMY installOnly
  bool ResObject::installOnly() const
  { return false; }

  Date ResObject::buildtime() const
  { return Date( lookupNumAttribute( sat::SolvAttr::buildtime ) ); }

  Date ResObject::installtime() const
  { return Date( lookupNumAttribute( sat::SolvAttr::installtime ) ); }

#warning DUMMY diskusage
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
