/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/solver/temporary/HelixPackageImpl.cc
 *
*/

#include "HelixPackageImpl.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"

using namespace std;
using namespace zypp::detail;
using namespace zypp::solver::detail;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////
    namespace detail
    {
      ///////////////////////////////////////////////////////////////////
      //
      //        CLASS NAME : HelixPackageImpl
      //
      ///////////////////////////////////////////////////////////////////

      /** Default ctor
      */
      HelixPackageImpl::HelixPackageImpl(
	const HelixParser & parsed
      )
      : _summary(parsed.summary),
	_description(parsed.description),
	_group(parsed.group),
	_install_only(parsed.installOnly),
        _size_package(parsed.fileSize),
	_size_installed(parsed.installedSize)
      {
      }

      /** Package summary */
      Label HelixPackageImpl::summary() const
      { return _summary; }

      /** Package description */
      Text HelixPackageImpl::description() const
      { return _description; }

      Text HelixPackageImpl::insnotify() const
      // metadata doesn't priovide this attribute
      { return ResObjectImplIf::insnotify(); }

      Text HelixPackageImpl::delnotify() const
      // metadata doesn't priovide this attribute
      { return ResObjectImplIf::delnotify(); }

      ByteCount HelixPackageImpl::size() const
#warning fixme
      { return 0; }

      bool HelixPackageImpl::providesSources() const
      { return ResObjectImplIf::providesSources(); }

      Label HelixPackageImpl::instSrcLabel() const
      { return ResObjectImplIf::instSrcLabel(); }
      
      Vendor HelixPackageImpl::instSrcVendor() const
      { return ResObjectImplIf::instSrcVendor(); }
      
      /** */
      Date HelixPackageImpl::buildtime() const
      { return _buildtime; }

      /** */
      std::string HelixPackageImpl::buildhost() const
      { return _buildhost; }

      /** */
      Date HelixPackageImpl::installtime() const
      { return PackageImplIf::installtime(); }

      /** */
      std::string HelixPackageImpl::distribution() const
#warning fixme
      { return string(); }

      /** */
      Vendor HelixPackageImpl::vendor() const
      { return _vendor; }

      /** */
      Label HelixPackageImpl::license() const
      { return _license; }

      /** */
      std::string HelixPackageImpl::packager() const
      { return _packager; }

      /** */
      PackageGroup HelixPackageImpl::group() const
      { return _group; }

      /** */
      Changelog HelixPackageImpl::changelog() const
      { return _changelog; }

      /** Don't ship it as class Url, because it might be
       * in fact anything but a legal Url. */
      std::string HelixPackageImpl::url() const
      { return _url; }

      /** */
      std::string HelixPackageImpl::os() const
      // metadata doesn't priovide this attribute
      { return PackageImplIf::os(); }

      /** */
      Text HelixPackageImpl::prein() const
      // metadata doesn't priovide this attribute
      { return PackageImplIf::prein(); }

      /** */
      Text HelixPackageImpl::postin() const
      // metadata doesn't priovide this attribute
      { return PackageImplIf::postin(); }

      /** */
      Text HelixPackageImpl::preun() const
      // metadata doesn't priovide this attribute
      { return PackageImplIf::preun(); }

      /** */
      Text HelixPackageImpl::postun() const
      // metadata doesn't priovide this attribute
      { return PackageImplIf::postun(); }

      /** */
      ByteCount HelixPackageImpl::sourcesize() const
#warning fixme
      { return 0; }

      /** */
      ByteCount HelixPackageImpl::archivesize() const
#warning fixme
      { return 0; }

      /** */
      Text HelixPackageImpl::authors() const
      { return _authors; }

      /** */
      Text HelixPackageImpl::filenames() const
      { return _filenames; }

      License HelixPackageImpl::licenseToConfirm() const
      { return _license_to_confirm; }

      /** */
      std::string HelixPackageImpl::type() const
      { return _type; }

      /** */
      std::list<std::string> HelixPackageImpl::keywords() const
      { return _keywords; }

      bool HelixPackageImpl::installOnly() const
      { return _install_only; }

      unsigned HelixPackageImpl::mediaId() const
      { return _mediaid; }

      CheckSum HelixPackageImpl::checksum() const
      { return _checksum; }

      std::list<DeltaRpm> HelixPackageImpl::deltaRpms() const
      { return _delta_rpms; }

      std::list<PatchRpm> HelixPackageImpl::patchRpms() const
      { return _patch_rpms; }

#if 0
      /** */
      std::list<std::string> HelixPackageImpl::insnotify() const
      { return std::list<std::string>(); }
      /** */
      std::list<std::string> HelixPackageImpl::delnotify() const
      { return std::list<std::string>(); }
      /** */
      unsigned HelixPackageImpl::packageSize() const
      { return _size_package; }
      /** */
      unsigned HelixPackageImpl::archiveSize() const
      { return _size_archive; }
      /** */
      unsigned HelixPackageImpl::installedSize() const
      { return _size_installed; }
// FIXME do not understand items below
      /** */
      bool HelixPackageImpl::providesSources() const
      {
	return false;
      }
      /** */
      std::string HelixPackageImpl::instSrcLabel() const
      {
	return "";
      }
      /** */
      std::string HelixPackageImpl::instSrcVendor() const
      {
	return "";
      }
      /** */
      unsigned HelixPackageImpl::instSrcRank() const
      {
	return 0;
      }
      /** */
      std::string HelixPackageImpl::buildhost() const
      {
	return _buildhost;
      }
      /** */
      std::string HelixPackageImpl::distribution() const
      {
	return "";
      }
      /** */
      std::string HelixPackageImpl::vendor() const
      {
	return _vendor;
      }
      /** */
      std::string HelixPackageImpl::license() const
      {
	return _license;
      }
      /** */
      std::list<std::string> HelixPackageImpl::licenseToConfirm() const
      {
	return std::list<std::string>();
      }
      /** */
      std::string HelixPackageImpl::packager() const
      {
	return _packager;
      }
      /** */
      std::string HelixPackageImpl::group() const
      {
	return _group;
      }
      /** */
      std::list<std::string> HelixPackageImpl::changelog() const
      {}
      /** */
      std::string HelixPackageImpl::url() const
      {
	return _url;
      }
      /** */
      std::string HelixPackageImpl::os() const
      {}
      /** */
      std::list<std::string> HelixPackageImpl::prein() const
      {}
      /** */
      std::list<std::string> HelixPackageImpl::postin() const
      {}
      /** */
      std::list<std::string> HelixPackageImpl::preun() const
      {}
      /** */
      std::list<std::string> HelixPackageImpl::postun() const
      {}
      /** */
      std::string HelixPackageImpl::sourcepkg() const
      { return _sourcepkg; }
      /** */
      std::list<std::string> HelixPackageImpl::authors() const
      { return _authors; }
      /** */
      std::list<std::string> HelixPackageImpl::filenames() const
      {}
      /** */
      std::list<std::string> HelixPackageImpl::recommends() const
      {}
      /** */
      std::list<std::string> HelixPackageImpl::suggests() const
      {}
      /** */
      std::string HelixPackageImpl::location() const
      {}
      /** */
      std::string HelixPackageImpl::md5sum() const
      {}
      /** */
      std::string HelixPackageImpl::externalUrl() const
      {}
      /** */
      std::list<Edition> HelixPackageImpl::patchRpmBaseVersions() const
      {}
      /** */
      unsigned HelixPackageImpl::patchRpmSize() const
      {}
      /** */
      bool HelixPackageImpl::forceInstall() const
      {}
      /** */
      std::string HelixPackageImpl::patchRpmMD5() const
      {}
      /** */
      bool HelixPackageImpl::isRemote() const
      {}
      /** */
      bool HelixPackageImpl::prefererCandidate() const
      {}

#endif

    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
