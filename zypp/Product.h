/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Product.h
 *
*/
#ifndef ZYPP_PRODUCT_H
#define ZYPP_PRODUCT_H

#include "zypp/Resolvable.h"
#include <list>
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    DEFINE_PTR_TYPE(ProductImpl)
    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  DEFINE_PTR_TYPE(Product)

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Product
  //
  /** Class representing an update script */
  class Product : public Resolvable
  {
  public:
    /** Default ctor */
    Product( detail::ProductImplPtr impl_r );
    /** Dtor */
    ~Product();
  public:
    /** Get the product description */
    std::list<std::string> description() const;
    /** Get the product categoty (base, add-on) */
    std::string category();
  private:
    /** Pointer to implementation */
    detail::ProductImplPtr _pimpl;
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PRODUCT_H
