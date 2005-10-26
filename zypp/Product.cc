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
#include "zypp/detail/ProductImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  IMPL_PTR_TYPE(Product)

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Product::Product
  //	METHOD TYPE : Ctor
  //
  Product::Product( detail::ProductImplPtr impl_r )
  : Resolvable( impl_r )
  , _pimpl( impl_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Product::~Product
  //	METHOD TYPE : Dtor
  //
  Product::~Product()
  {}

  std::list<std::string> Product::description() const
  {
    return _pimpl->description();
  }
  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Product::do_script
  //	Get the product category
  //
  std::string Product::category()
  {
    return _pimpl->category();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
