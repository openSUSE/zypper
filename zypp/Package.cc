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

  DiskUsage Package::diskusage() const
  { return pimpl().diskusage(); }

  /** */
  std::list<std::string> Package::authors() const
  { return pimpl().authors(); }

  /** */
  std::list<std::string> Package::filenames() const
  { return pimpl().filenames(); }

  /** */
  License Package::licenseToConfirm() const
  { return pimpl().licenseToConfirm(); }

  /** */
  Pathname Package::plainRpm() const
  { return pimpl().location(); }

  /** */
  std::list<PatchRpm> Package::patchRpms() const
  { return pimpl().patchRpms(); }

  /** */
  std::list<DeltaRpm> Package::deltaRpms() const
  { return pimpl().deltaRpms(); }

  /** */
  Pathname Package::getPlainRpm() const
  { return source().provideFile(plainRpm(), mediaId()); }

  /** */
  Pathname Package::getDeltaRpm(BaseVersion & base_r) const
  {
#warning (2x): TODO: BaseVersion compare checks only Edition
    std::list<DeltaRpm> deltas = deltaRpms();
    for (std::list<DeltaRpm>::const_iterator it = deltas.begin();
         it != deltas.end(); it++)
    {
      if (base_r == it->baseVersion())
	return source().provideFile(it->filename());
    }
    ZYPP_THROW(Exception("No matching delta RPM found"));
    return source().provideFile(plainRpm()); // never reached
  }

  /** */
  Pathname Package::getPatchRpm(BaseVersion & base_r) const
  {
    std::list<PatchRpm> patches = patchRpms();
    for (std::list<PatchRpm>::const_iterator it = patches.begin();
         it != patches.end(); it++)
    {
      std::list<BaseVersion> bases = it->baseVersions();
      for (std::list<BaseVersion>::const_iterator bvit = bases.begin();
	   bvit != bases.end(); bvit++)
      {
	if (base_r == *bvit)
	  return source().provideFile(it->filename());
      }
    }
    ZYPP_THROW(Exception("No matching patch RPM found"));
    return source().provideFile(plainRpm()); // never reached
  }

  bool Package::installOnly() const
  { return pimpl().installOnly(); }

  unsigned Package::mediaId() const
  { return pimpl().mediaId(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
