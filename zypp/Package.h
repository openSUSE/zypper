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
#include "zypp/detail/PackageImplIf.h"

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
    typedef detail::PackageImplIf    Impl;
    typedef Package                  Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /**
     * Checksum the source says this package should have
     */
    CheckSum checksum() const;
    /** Time of package installation */
    Date installtime() const;
    /** Get the package change log */
    Changelog changelog() const;
    /** */
    Date buildtime() const;
    /** */
    std::string buildhost() const;
    /** */
    std::string distribution() const;
    /** */
    Vendor vendor() const;
    /** */
    Label license() const;
    /** */
    std::string packager() const;
    /** */
    PackageGroup group() const;
    /** Don't ship it as class Url, because it might be
     * in fact anything but a legal Url. */
    std::string url() const;
    /** */
    std::string os() const;
    /** */
    Text prein() const;
    /** */
    Text postin() const;
    /** */
    Text preun() const;
    /** */
    Text postun() const;
    /** */
    ByteCount sourcesize() const;
    /** */
    ByteCount archivesize() const;
    /** */
    std::list<std::string> authors() const;
    /** */
    std::list<std::string> filenames() const;

    /** Disk usage per directory */
    DiskUsage diskusage() const;

    /** */
    License licenseToConfirm() const;

    /** */
    bool installOnly() const;
    /** ID of the media */
    unsigned mediaId() const;
    /** location in source */
    Pathname location() const;

    // data here:


  protected:
    Package( const NVRAD & nvrad_r );
    /** Dtor */
    virtual ~Package();

  private:
    /** Access implementation */
    virtual Impl & pimpl() = 0;
    /** Access implementation */
    virtual const Impl & pimpl() const = 0;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PACKAGE_H
