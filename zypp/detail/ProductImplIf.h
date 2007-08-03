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

#include "zypp/Locale.h"
#include "zypp/Url.h"
#include "zypp/detail/ResObjectImplIf.h"

#ifndef PURE_VIRTUAL
#define PURE_VIRTUAL = 0
#endif

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
    /** Abstract Product implementation interface.
    */
    class ProductImplIf : public ResObjectImplIf
    {
    public:
      typedef Product ResType;

    public:
      /** Get the type of the product - addon or base*/
      virtual std::string type() const PURE_VIRTUAL;

      virtual Url releaseNotesUrl() const PURE_VIRTUAL;

      virtual std::list<Url> updateUrls() const PURE_VIRTUAL;

      /**
       * Additional software for the product
       * They are complementary, not alternatives.
       */
      virtual std::list<Url> extraUrls() const  PURE_VIRTUAL;

      /**
       * Optional software for the product
       * (for example. Non OSS repositories)
       * They are complementary, not alternatives.
       */
      virtual std::list<Url> optionalUrls() const  PURE_VIRTUAL;

      /** The product flags */
      virtual std::list<std::string> flags() const PURE_VIRTUAL;

      virtual TranslatedText shortName() const PURE_VIRTUAL;

      /** Vendor specific distribution id. */
      virtual std::string distributionName() const PURE_VIRTUAL;

      /** Vendor specific distribution version. */
      virtual Edition distributionEdition() const PURE_VIRTUAL;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PRODUCTIMPLIF_H
