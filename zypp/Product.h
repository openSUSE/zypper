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

#include "zypp/ResObject.h"
#include "zypp/detail/ProductImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResObject
  //
  /** Product interface.
  */
  class Product : public ResObject
  {
  public:
    typedef detail::ProductImplIf    Impl;
    typedef Product                  Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /** Get the product categoty (base, add-on) */
    std::string category() const;

  protected:
    /** Ctor */
    Product( const NVRAD & nvrad_r );
    /** Dtor */
    virtual ~Product();

  private:
    /** Access implementation */
    virtual Impl & pimpl() = 0;
    /** Access implementation */
    virtual const Impl & pimpl() const = 0;
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PRODUCT_H
