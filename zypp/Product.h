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

    /** For installed products the name of the coddesponding
     * \c /etc/products.d entry.
    .*/
    std::string referenceFilename() const;

    /** List of packages included in older versions of this product and now dropped.
     *
     * This evaluates the \ref referencePackage \c weakremover namespace. It actually
     * returns a \ref CapabilitySet, because we support to drop specific versions or
     * version ranges of a package. Use \ref sat::WhatProvides to get the actually
     * installed and available packages matching this list.
     * \code
     *   const Product & openSUSE;
     *   sat::WhatProvides dropped( openSUSE.droplist() );
     *   for_( it, dropped.poolItemBegin(), dropped.poolItemEnd() )
     *   {
     *     if ( it->status().isInstalled() )
     *     {
     *       MIL << "Installed but no longer supported package: " << *it << endl;
     *     }
     *   }
     * \endcode
     */
    CapabilitySet droplist() const;

  public:
    /***/
    typedef std::vector<constPtr> ReplacedProducts;

    /** Array of \b installed Products that would be replaced by
     *  installing this one.
     */
    ReplacedProducts replacedProducts() const;

    /** Vendor specific string denoting the product line. */
    std::string productLine() const;

  public:
    /** Untranslated short name like <tt>SLES 10</tt> (fallback: name) */
    std::string shortName() const;

    /** The product flavor (LiveCD Demo, FTP edition,...). */
    std::string flavor() const;

    /** Get the product type
     * Well, in an ideal world there is only one base product.
     * It's the installed product denoted by a symlink in
     * \c /etc/products.d.
     */
    std::string type() const;

    /** The product flags */
    std::list<std::string> flags() const;

  public:
    /** This is the \b installed product that is also targeted by the
     *  \c /etc/products.d/baseproduct symlink.
    */
    bool isTargetDistribution() const;

    /** This is \c register.target attribute of an \b installed product.
      * Used for registration.
      */
    std::string registerTarget() const;

    /** This is \c register.release attribute of an \b installed product.
      * Used for registration.
      */
    std::string registerRelease() const;

  public:
    /***/
    class UrlList;

    /** Rerieve urls flagged with \c key_r for this product.
     *
     * This is the most common interface. There are convenience methods for
     * wellknown flags like \c "releasenotes", \c "register", \c "updateurls",
     * \c "extraurls", \c "optionalurls" and \c "smolt" below.
     */
    UrlList urls( const std::string & key_r ) const;

    /** The URL to download the release notes for this product. */
    UrlList releaseNotesUrls() const;

    /** The URL for registration. */
    UrlList registerUrls() const;

    /** The URL for SMOLT \see http://smolts.org/wiki/Main_Page. */
    UrlList smoltUrls() const;

    /**
     * Online updates for the product.
     * They are complementary, not alternatives. #163192
     */
    UrlList updateUrls() const;

    /**
     * Additional software for the product
     * They are complementary, not alternatives.
     */
    UrlList extraUrls() const;

    /**
     * Optional software for the product.
     * (for example. Non OSS repositories)
     * They are complementary, not alternatives.
     */
    UrlList optionalUrls() const;

  protected:
    friend Ptr make<Self>( const sat::Solvable & solvable_r );
    /** Ctor */
    Product( const sat::Solvable & solvable_r );
    /** Dtor */
    virtual ~Product();
  };

  /** Helper to iterate a products URL lists.
   * \ref first is a convenience for 'lists' with just
   * one entry (e.g. releaseNotesUrls)
   */
  class Product::UrlList
  {
    private:
      /** \todo Change to directly iterate the .solv */
      typedef std::list<Url> ListType;

    public:
      typedef ListType::value_type     value_type;
      typedef ListType::size_type      size_type;
      typedef ListType::const_iterator const_iterator;

      bool empty() const
      { return _list.empty(); }

      size_type size() const
      { return _list.size(); }

      const_iterator begin() const
      { return _list.begin(); }

      const_iterator end() const
      { return _list.end(); }

      /** The first Url or an empty Url. */
      Url first() const
      { return empty() ? value_type() : _list.front(); }

    public:
      /** The key used to retrieve this list (for debug) */
      std::string key() const
      { return _key; }

    private:
      friend class Product;
      /** Change to directly iterate the .solv */
      std::string _key;
      ListType    _list;
  };

  /** \relates Product::UrlList Stream output. */
  std::ostream & operator<<( std::ostream & str, const Product::UrlList & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PRODUCT_H
