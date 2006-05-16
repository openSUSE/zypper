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
#include "zypp/source/SourceImpl.h"
#include "zypp/base/Exception.h"

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

  /** */
  std::string Package::buildhost() const
  { return pimpl().buildhost(); }

  CheckSum Package::checksum() const
  { return pimpl().checksum(); }

  /** */
  std::string Package::distribution() const
  { return pimpl().distribution(); }

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

  DiskUsage Package::diskusage() const
  { return pimpl().diskusage(); }

  /** */
  std::list<std::string> Package::authors() const
  { return pimpl().authors(); }

  /** */
  std::list<std::string> Package::filenames() const
  { return pimpl().filenames(); }

  Pathname Package::location() const
  { return pimpl().location(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
