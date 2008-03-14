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

using namespace std;

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

#warning DUMMY changelog
  Changelog Package::changelog() const
  { return Changelog(); }

  /** */
  std::string Package::buildhost() const
  { return lookupStrAttribute( sat::SolvAttr::buildhost ); }

  /** */
  std::string Package::distribution() const
  { return lookupStrAttribute( sat::SolvAttr::distribution ); }

  /** */
  Label Package::license() const
  { return lookupStrAttribute( sat::SolvAttr::eula ); }

  /** */
  std::string Package::packager() const
  { return lookupStrAttribute( sat::SolvAttr::packager ); }

  /** */
  PackageGroup Package::group() const
  { return lookupStrAttribute( sat::SolvAttr::group ); }

#warning DUMMY keywords
  Package::Keywords Package::keywords() const
  { return Keywords(); }

  /** Don't ship it as class Url, because it might be
   * in fact anything but a legal Url. */
#warning DUMMY url
  std::string Package::url() const
  { return string(); }

  /** */
  std::string Package::os() const
  { return lookupStrAttribute( sat::SolvAttr::os ); }

  /** */
  Text Package::prein() const
  { return lookupStrAttribute( sat::SolvAttr::prein); }

  /** */
  Text Package::postin() const
  { return lookupStrAttribute( sat::SolvAttr::postin); }

  /** */
  Text Package::preun() const
  { return lookupStrAttribute( sat::SolvAttr::preun); }

  /** */
  Text Package::postun() const
  { return lookupStrAttribute( sat::SolvAttr::postun); }

  /** */
  ByteCount Package::sourcesize() const
  { return lookupNumAttribute( sat::SolvAttr::sourcesize); }

  /** */
#warning DUMMY authors
  std::list<std::string> Package::authors() const
  { return std::list<std::string>(); }

  /** */
#warning DUMMY filenames
  std::list<std::string> Package::filenames() const
  { return std::list<std::string>(); }

  OnMediaLocation Package::location() const
  {
#warning MISSING checkdums in OnMediaLocation
    OnMediaLocation loc;
    unsigned medianr;
    std::string filename = lookupLocation( medianr );
    /* XXX This datadir should be part of RepoInfo.  */
    if (repoInfo().type().toEnum() == repo::RepoType::YAST2_e)
      filename = std::string("suse/") + filename;
    loc.setLocation(filename, medianr);
    return loc;
  }

#warning DUMMY sourcePkgName
 std::string Package::sourcePkgName() const
  { return std::string(); }

#warning DUMMY sourcePkgEdition
 Edition Package::sourcePkgEdition() const
  { return Edition(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
