/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SrcPackage.cc
 *
*/
#include "zypp/SrcPackage.h"
///////////////////////////////////////////////////////////////////
namespace zyppintern
{
  using namespace zypp;
  // in Package.cc
  Pathname cachedLocation( const OnMediaLocation & loc_r, const RepoInfo & repo_r );
} // namespace zyppintern
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(SrcPackage);

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SrcPackage::SrcPackage
  //	METHOD TYPE : Ctor
  //
  SrcPackage::SrcPackage( const sat::Solvable & solvable_r )
  : ResObject( solvable_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SrcPackage::~SrcPackage
  //	METHOD TYPE : Dtor
  //
  SrcPackage::~SrcPackage()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	SrcPackage interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  std::string SrcPackage::sourcePkgType() const
  { return lookupStrAttribute( sat::SolvAttr::arch ); }

  OnMediaLocation SrcPackage::location() const
  { return lookupLocation(); }

  Pathname SrcPackage::cachedLocation() const
  { return zyppintern::cachedLocation( location(), repoInfo() ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
