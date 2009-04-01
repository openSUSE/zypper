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

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
