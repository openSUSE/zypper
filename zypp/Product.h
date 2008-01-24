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

#include <list>
#include <string>

#include "zypp/ResObject.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Product);

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResObject
  //
  /** Product interface.
  */
  class Product : public ResObject
  {
  public:
    typedef Product                  Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /** Get the product type (base, add-on) */
    std::string type() const;

    /** Get the vendor of the product */
    Label vendor() const;

    /** The URL to download the release notes for this product */
    Url releaseNotesUrl() const;

    /**
     * Online updates for the product.
     * They are complementary, not alternatives. #163192
     */
    std::list<Url> updateUrls() const;

    /**
     * Additional software for the product
     * They are complementary, not alternatives.
     */
    std::list<Url> extraUrls() const;

    /**
     * Optional software for the product
     * (for example. Non OSS repositories)
     * They are complementary, not alternatives.
     */
    std::list<Url> optionalUrls() const;

    /** The product flags */
    std::list<std::string> flags() const;

    /** */
    Label shortName() const;

    /** */
    Label longName() const;

    /** Vendor specific distribution id. */
    std::string distributionName() const;

    /** Vendor specific distribution version. */
    Edition distributionEdition() const;

  protected:
    friend Ptr make<Self>( const sat::Solvable & solvable_r );
    /** Ctor */
    Product( const sat::Solvable & solvable_r );
    /** Dtor */
    virtual ~Product();
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PRODUCT_H
