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

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Package::Package
  //	METHOD TYPE : Ctor
  //
  Package::Package( const NVRAD & nvrad_r )
  : ResObject( TraitsType::kind, nvrad_r )
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
  { return pimpl().changelog(); }

  /** Time of package installation */
  Date Package::installtime() const
  { return pimpl().installtime(); }

  /** */
  Date Package::buildtime() const
  { return pimpl().buildtime(); }

  /** */
  std::string Package::buildhost() const
  { return pimpl().buildhost(); }

  /** */
  std::string Package::distribution() const
  { return pimpl().distribution(); }

    /** */
  Vendor Package::vendor() const
  { return pimpl().vendor(); }

  /** */
  Label Package::license() const
  { return pimpl().license(); }

  /** */
  std::string Package::packager() const
  { return pimpl().packager(); }

  /** */
  PackageGroup Package::group() const
  { return pimpl().group(); }


  /** Don't ship it as class Url, because it might be
   * in fact anything but a legal Url. */
  std::string Package::url() const
  { return pimpl().url(); }

  /** */
  std::string Package::os() const
  { return pimpl().os(); }

  /** */
  Text Package::prein() const
  { return pimpl().prein(); }

  /** */
  Text Package::postin() const
  { return pimpl().postin(); }

  /** */
  Text Package::preun() const
  { return pimpl().preun(); }

  /** */
  Text Package::postun() const
  { return pimpl().postun(); }

  /** */
  ByteCount Package::sourcesize() const
  { return pimpl().sourcesize(); }

  /** */
  ByteCount Package::archivesize() const
  { return pimpl().archivesize(); }

  /** */
  Text Package::authors() const
  { return pimpl().authors(); }

  /** */
  Text Package::filenames() const
  { return pimpl().filenames(); }

  /** */
  License Package::licenseToConfirm() const
  { return pimpl().licenseToConfirm(); }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
