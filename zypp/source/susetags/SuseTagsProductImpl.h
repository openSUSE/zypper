/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/ProductImpl.h
 *
*/
#ifndef ZYPP_DETAIL_SUSETAGS_PRODUCTIMPL_H
#define ZYPP_DETAIL_SUSETAGS_PRODUCTIMPL_H

#include "zypp/detail/ProductImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
  namespace susetags
  {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ProductImpl
    //
    /**
    */
    struct SuseTagsProductImpl : public zypp::detail::ProductImplIf
    {
    public:
      SuseTagsProductImpl();
      virtual ~SuseTagsProductImpl();

      virtual std::string category() const;
      virtual Label vendor() const;
      virtual Label displayName() const;

      std::string _vendor;
      std::string _category;
      std::string _displayName;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace susetags
  ///////////////////////////////////////////////////////////////////
  } // namespace source
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PRODUCTIMPL_H
