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
     * Checksum the source says this package should have.
     * \see \ref location
     */
    CheckSum checksum() const
    { return location().checksum(); }

    /** Location of the resolvable in the repository.
     * \ref OnMediaLocation conatins all information required to
     * retrieve the packge (url, checksum, etc.).
     */
    OnMediaLocation location() const;


    /** \deprecated no metadata always empty */
    ZYPP_DEPRECATED std::string os() const { return std::string(); }
    /** \deprecated no metadata always empty */
    ZYPP_DEPRECATED std::string prein() const { return std::string(); }
    /** \deprecated no metadata always empty */
    ZYPP_DEPRECATED std::string postin() const { return std::string(); }
    /** \deprecated no metadata always empty */
    ZYPP_DEPRECATED std::string preun() const { return std::string(); }
    /** \deprecated no metadata always empty */
    ZYPP_DEPRECATED std::string postun() const { return std::string(); }

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
