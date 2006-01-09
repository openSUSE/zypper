/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Package.cc
 *
*/
#include "zypp/Package.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Package::Package
  //	METHOD TYPE : Ctor
  //
  Package::Package( const NVRAD & nvrad_r )
  : ResObject( TraitsType::kind, nvrad_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Package::~Package
  //	METHOD TYPE : Dtor
  //
  Package::~Package()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	Package interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  Changelog Package::changelog()
  { return pimpl().changelog(); }

  /** Time of package installation */
  Date Package::installtime()
  { return pimpl().installtime(); }
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
