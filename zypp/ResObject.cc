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
#include "zypp/sat/SolvAttr.h"
#include "zypp/sat/Solvable.h"
#include "zypp/Repository.h"
#include "zypp/RepoInfo.h"
#include "zypp/IdString.h"

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

  std::string ResObject::summary( const Locale & lang_r ) const
  { return lookupStrAttribute( sat::SolvAttr::summary, lang_r ); }

  std::string ResObject::description( const Locale & lang_r ) const
  { return lookupStrAttribute( sat::SolvAttr::description, lang_r ); }

  std::string ResObject::insnotify( const Locale & lang_r ) const
  { return lookupStrAttribute( sat::SolvAttr::insnotify, lang_r ); }

  std::string ResObject::delnotify( const Locale & lang_r ) const
  { return lookupStrAttribute( sat::SolvAttr::delnotify, lang_r ); }

  std::string ResObject::licenseToConfirm( const Locale & lang_r ) const
  {
    std::string ret = lookupStrAttribute( sat::SolvAttr::eula, lang_r );
    if ( ret.empty() && isKind<Product>() )
      return repoInfo().getLicense( lang_r );
    return ret;
  }

  bool ResObject::needToAcceptLicense() const
  {
    if ( isKind<Product>() )
      return repoInfo().needToAcceptLicense( );
    return true;
  }

  std::string ResObject::distribution() const
  { return lookupStrAttribute( sat::SolvAttr::distribution ); }

  std::string ResObject::cpeId() const
  { return lookupStrAttribute( sat::SolvAttr::cpeid ); }

  ByteCount ResObject::installSize() const
  { return ByteCount( lookupNumAttribute( sat::SolvAttr::installsize ) ); }

  ByteCount ResObject::downloadSize() const
  { return ByteCount( lookupNumAttribute( sat::SolvAttr::downloadsize ) ); }

  unsigned ResObject::mediaNr() const
  { return lookupNumAttribute( sat::SolvAttr::medianr ); }

  Date ResObject::buildtime() const
  { return Date( lookupNumAttribute( sat::SolvAttr::buildtime ) ); }

  Date ResObject::installtime() const
  { return Date( lookupNumAttribute( sat::SolvAttr::installtime ) ); }

   /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#include "zypp/ResObjects.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

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

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
