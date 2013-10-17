/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Package.h
 *
*/
#ifndef ZYPP_PACKAGE_H
#define ZYPP_PACKAGE_H

#include "zypp/ResObject.h"
#include "zypp/PackageKeyword.h"
#include "zypp/Changelog.h"
#include "zypp/VendorSupportOptions.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Package);

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Package
  //
  /** Package interface.
  */
  class Package : public ResObject
  {
  public:
    typedef Package                  Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    typedef sat::ArrayAttr<PackageKeyword,IdString> Keywords;
    typedef sat::ArrayAttr<std::string,std::string> FileList;

  public:

    /**
     * Returns the level of supportability the vendor
     * gives to this package.
     *
     * If the identical package happens to appear in multiple
     * repos with different support levels, the maximum
     * level is returned.
     *
     * This is one value from \ref VendorSupportOption.
     */
    VendorSupportOption vendorSupport() const;

    /**
     * True if the vendor support for this package
     * is unknown or explictly unsupported.
     */
    bool maybeUnsupported() const;

    /** Get the package change log */
    Changelog changelog() const;
    /** */
    std::string buildhost() const;
    /** */
    std::string distribution() const;
    /** */
    std::string license() const;
    /** */
    std::string packager() const;
    /** */
    std::string group() const;
    /** */
    Keywords keywords() const;
    /** Don't ship it as class Url, because it might be
     * in fact anything but a legal Url. */
    std::string url() const;
    /** Size of corresponding the source package. */
    ByteCount sourcesize() const;
    /** */
    std::list<std::string> authors() const;

    /** Return the packages filelist (if available).
     * The returned \ref FileList appears to be a container of
     * \c std::string. In fact it is a query, so it does not
     * consume much memory.
    */
    FileList filelist() const;

    /** \name Source package handling
    */
    //@{
    /** Name of the source rpm this package was built from.
     */
    std::string sourcePkgName() const;

    /** Edition of the source rpm this package was built from.
     */
    Edition sourcePkgEdition() const;

    /** The type of the source rpm (\c "src" or \c "nosrc").
     */
    std::string sourcePkgType() const;

    /** The source rpms \c "name-version-release.type"
     */
    std::string sourcePkgLongName() const;
    //@}

    /**
     * Checksum the source says this package should have.
     * \see \ref location
     */
    CheckSum checksum() const;

    /** Location of the resolvable in the repository.
     * \ref OnMediaLocation conatins all information required to
     * retrieve the packge (url, checksum, etc.).
     */
    OnMediaLocation location() const;

    /** Location of the downloaded package in cache or an empty path. */
    Pathname cachedLocation() const;

    /** Whether the package is cached. */
    bool isCached() const
    { return ! cachedLocation().empty(); }

  protected:
    friend Ptr make<Self>( const sat::Solvable & solvable_r );
    /** Ctor */
    Package( const sat::Solvable & solvable_r );
    /** Dtor */
    virtual ~Package();
  };

  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PACKAGE_H
