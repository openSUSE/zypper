/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/ProductImplIf.h
 *
*/
#ifndef ZYPP_DETAIL_PRODUCTIMPLIF_H
#define ZYPP_DETAIL_PRODUCTIMPLIF_H

#include "zypp/detail/ResObjectImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Product;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ProductImplIf
    //
    /** Abstact Product implementation interface.
    */
    class ProductImplIf : public ResObjectImplIf
    {
    public:
      typedef Product ResType;

    public:
#if 0
      /** Get the category of the product */
      virtual std::string category() const = 0;
      virtual Label vendor() const = 0;
      virtual Label displayName() const = 0;
#endif
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PRODUCTIMPLIF_H
