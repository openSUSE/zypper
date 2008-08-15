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
  //	CLASS NAME : Product
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
    /** The reference package providing the product metadata,
     *  if such a package exists.
     */
    sat::Solvable referencePackage() const;

  public:
    /***/
    typedef std::vector<constPtr> ReplacedProducts;

    /** Array of \b installed Products that would be replaced by
     *  installing this one.
     */
    ReplacedProducts replacedProducts() const;

  public:
    /** Untranslated short name like <tt>SLES 10</tt>*/
    std::string shortName() const;

    /** The product flavor (LiveCD Demo, FTP edition,...). */
    std::string flavor() const;

    /** Get the product type (base, add-on)
     * Well, in an ideal world there is only one base product.
     * It's the installed product denoted by a symlink in
     * \c /etc/products.d.
    */
    std::string type() const;

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

    /** Translated long name like <tt>SUSE Linux Enterprise Server 10</tt>
     * \deprecated use summary.
     */
    std::string longName( const Locale & lang_r = Locale() ) const ZYPP_DEPRECATED
    { return summary( lang_r ); }

    /** Vendor specific distribution id.
     * \deprecated replaced by ResObject::distribution
     */
    std::string distributionName() const ZYPP_DEPRECATED;

    /** Vendor specific distribution version.
     * \deprecated replaced by ResObject::distribution
     */
    Edition distributionEdition() const ZYPP_DEPRECATED;

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
