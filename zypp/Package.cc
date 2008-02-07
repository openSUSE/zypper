/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Package.cc
 *
*/
#include "zypp/Package.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(Package);

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Package::Package
  //	METHOD TYPE : Ctor
  //
  Package::Package( const sat::Solvable & solvable_r )
  : ResObject( solvable_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Package::~Package
  //	METHOD TYPE : Dtor
  //
  Package::~Package()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	Package interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  Changelog Package::changelog() const
  { return Changelog(); }

  /** */
  std::string Package::buildhost() const
  { return std::string(); }

  /** */
  std::string Package::distribution() const
  { return std::string(); }

  /** */
  Label Package::license() const
  { return Label(); }

  /** */
  std::string Package::packager() const
  { return std::string(); }

  /** */
  PackageGroup Package::group() const
  { return PackageGroup(); }

  Package::Keywords Package::keywords() const
  { return Keywords(); }

  /** Don't ship it as class Url, because it might be
   * in fact anything but a legal Url. */
  std::string Package::url() const
  { return std::string(); }

  /** */
  std::string Package::os() const
  { return std::string(); }

  /** */
  Text Package::prein() const
  { return Text(); }

  /** */
  Text Package::postin() const
  { return Text(); }

  /** */
  Text Package::preun() const
  { return Text(); }

  /** */
  Text Package::postun() const
  { return Text(); }

  /** */
  ByteCount Package::sourcesize() const
  { return ByteCount(); }

  /** */
  std::list<std::string> Package::authors() const
  { return std::list<std::string>(); }

  /** */
  std::list<std::string> Package::filenames() const
  { return std::list<std::string>(); }

  OnMediaLocation Package::location() const
  {
    OnMediaLocation loc;
    unsigned medianr;
    std::string filename = lookupLocation( medianr );
    /* XXX someone else needs to do this prepending of the datadir.
       It's not necessarily "suse".  */
    filename = "suse/" + filename;
    loc.setLocation(filename, medianr);
    return loc;
  }

  std::string Package::sourcePkgName() const
  { return std::string(); }

  Edition Package::sourcePkgEdition() const
  { return Edition(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
