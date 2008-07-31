/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/ProductInfo.h
 *
*/
#ifndef ZYPP_PRODUCTINFO_H
#define ZYPP_PRODUCTINFO_H

#include <iosfwd>

#include "zypp/sat/Solvable.h"
#include "zypp/Locale.h"
#include "zypp/Vendor.h"
#include "zypp/Repository.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Url;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ProductInfo
  //
  /** Product metadata.
   *
   * \ref ProductInfo allows to retrieve product related metadata, that
   * might be associated with certain packages. This class basically
   * provides the information stored in an \c /etc/products.d entry.
   *
   * \ref ProductInfo is provided by \ref Package.
   *
   * The underlying \c private \ref sat::Solvable either refers to
   * an associated package providing product related metadata, or
   * is \ref sat::noSolvable.
   */
  class ProductInfo : private sat::Solvable
  {
    public:
      /** Represents no \ref ProductInfo. */
      static const ProductInfo noProductInfo;

    public:
      /** Default ctor creates \ref noProductInfo. */
      ProductInfo();

      /** ProductInfo associated with a \ref sat::Solvable or \ref noProductInfo. */
      explicit ProductInfo( sat::Solvable solvable_r );

      /** ProductInfo associated with a \ref PoolItem or \ref noProductInfo. */
      explicit ProductInfo( const PoolItem & poolItem_r );

      /** ProductInfo associated with a \ref ResObject or \ref noProductInfo. */
      explicit ProductInfo( const ResTraits<ResObject>::constPtrType & package_r );

    public:
#ifndef SWIG // Swig treats it as syntax error
      /** Whether this represents a valid- or \ref noProductInfo. */
      using sat::Solvable::operator bool_type;
#endif

    public:
      /** \name Access the associated \ref Package.
       */
      //@{
      /** Access the associated \ref sat:::Solvable. */
      sat::Solvable satSolvable() const
      { return *this; }

      /** Access the associated \ref PoolItem. */
      PoolItem poolItem() const;

      /** Access the associated \ref Package. */
      ResTraits<Package>::constPtrType package() const;
      //@}

    public:
      /** \name Product metadata.
       */
      //@{
      /** Whether this represents an installed Product. */
      using sat::Solvable::isSystem;

      std::string  name()     const;
      Edition      edition()  const;
      Arch         arch()     const;
      Vendor       vendor()   const;

      std::string  summary( const Locale & lang_r = Locale() ) const;
      std::string  description( const Locale & lang_r = Locale() ) const;

      /** The \ref Repository this \ref Product belongs to. */
      using sat::Solvable::repository;

      /** \ref RepoInfo associated with the repository
       *  providing this product.
       */
      RepoInfo repoInfo() const { return repository().info(); }


      //----------------------------------------------
      // These are the old Product attibutes to be revieved:

      /** Get the product type (base, add-on) */
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

      /** Untranslated short name like <tt>SLES 10</tt>*/
      std::string shortName() const;

      /** Translated long name like <tt>SUSE Linux Enterprise Server 10</tt>*/
      std::string longName( const Locale & lang_r = Locale() ) const;

      /** Vendor specific distribution id. */
      std::string distributionName() const;

      /** Vendor specific distribution version. */
      Edition distributionEdition() const;
      //@}
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ProductInfo Stream output */
  std::ostream & operator<<( std::ostream & str, const ProductInfo & obj );

  /** \relates ProductInfo */
  inline bool operator==( const ProductInfo & lhs, const ProductInfo & rhs )
  { return lhs.satSolvable() == rhs.satSolvable(); }

  /** \relates ProductInfo */
  inline bool operator!=( const ProductInfo & lhs, const ProductInfo & rhs )
  { return lhs.satSolvable() != rhs.satSolvable(); }

  /** \relates ProductInfo */
  inline bool operator<( const ProductInfo & lhs, const ProductInfo & rhs )
  { return lhs.satSolvable() < rhs.satSolvable(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PRODUCTINFO_H
