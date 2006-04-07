/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Product.cc
 *
*/
#include <iostream>

#include "zypp/Product.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(Product);
  
  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Product::Product
  //	METHOD TYPE : Ctor
  //
  Product::Product( const NVRAD & nvrad_r )
  : ResObject( TraitsType::kind, nvrad_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Product::~Product
  //	METHOD TYPE : Dtor
  //
  Product::~Product()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	Package interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  std::string Product::category() const
  { return pimpl().category(); }

  Label Product::vendor() const
  { return pimpl().vendor(); }

  Url Product::releaseNotesUrl() const
  { return pimpl().releaseNotesUrl(); }

  std::list<std::string> Product::flags() const
  { return pimpl().flags(); }

  /** */
  License Product::licenseToConfirm() const
  { return pimpl().licenseToConfirm(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
