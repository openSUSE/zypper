/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/ProductImpl.h
 *
*/
#ifndef ZYPP_DETAIL_PRODUCTIMPL_H
#define ZYPP_DETAIL_PRODUCTIMPL_H

#include <list>
#include <string>

#include "zypp/detail/ResolvableImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    DEFINE_PTR_TYPE(ProductImpl)

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ProductImpl
    //
    /** Class representing an update script */
    class ProductImpl : public ResolvableImpl
    {
    public:
      /** Default ctor */
      ProductImpl( const std::string & name_r,
		  const Edition & edition_r,
		  const Arch & arch_r );
      /** Dtor */
      ~ProductImpl();

    public:
      /** Get the product description */
      std::list<std::string> description() const;
      /** Get the category of the product */
      std::string category() const;
    protected:
      /** Product description */
      std::list<std::string> _description;
      /** The category of the product */
      std::string _category;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PRODUCTIMPL_H
