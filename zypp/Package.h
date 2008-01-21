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
    typedef std::set<PackageKeyword> Keywords;

  public:

    /** Get the package change log */
    Changelog changelog() const;
    /** */
    std::string buildhost() const;
    /** */
    std::string distribution() const;
    /** */
    Label license() const;
    /** */
    std::string packager() const;
    /** */
    PackageGroup group() const;
    /** */
    Keywords keywords() const;
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
    std::list<std::string> authors() const;
    /** */
    std::list<std::string> filenames() const;

    /** Name of the source rpm this package was built from.
     * Empty if unknown.
     */
    std::string sourcePkgName() const;

    /** Edition of the source rpm this package was built from.
     * Empty if unknown.
     */
    Edition sourcePkgEdition() const;

    /**
     * Checksum the source says this package should have
     * \deprecated Use location().checksum()
     */
    ZYPP_DEPRECATED CheckSum checksum() const
    { return location().checksum(); }

    /**
     * \short Location of the resolvable in the repository
     */
    OnMediaLocation location() const;

  protected:
    Package( const sat::Solvable & solvable_r );
    /** Dtor */
    virtual ~Package();
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PACKAGE_H
