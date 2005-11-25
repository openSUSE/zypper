/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMPackageImpl.cc
 *
*/

#include "zypp/source/yum/YUMPackageImpl.h"
#include "zypp/base/String.h"

using namespace std;
using namespace zypp::detail;
using namespace zypp::parser::yum;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    namespace yum
    {
      ///////////////////////////////////////////////////////////////////
      //
      //        CLASS NAME : YUMPackageImpl
      //
      ///////////////////////////////////////////////////////////////////

      /** Default ctor
      */
      YUMPackageImpl::YUMPackageImpl(
	const zypp::parser::yum::YUMPrimaryData & parsed
      )
      : _summary(parsed.summary),
	_description(),
	_buildtime(strtol(parsed.timeBuild.c_str(), 0, 10)),
	_buildhost(parsed.buildhost),
	_url(parsed.url),
	_vendor( parsed.vendor),
	_license( parsed.license),
	_packager(parsed.packager),
	_group(parsed.group),
	_changelog(), // TODO
	_type(parsed.type),
	_license_to_confirm(), // TODO add to metadata
	_authors(parsed.authors),
	_keywords( parsed.keywords),
	_mediaid(strtol(parsed.media.c_str(), 0, 10)),
	_checksum(parsed.checksumType,
		  parsed.checksum),
	_filenames(),
	_location(parsed.location),

	_install_only(parsed.installOnly)
#if 0
      : _size_package(strtol(parsed.sizePackage.c_str(), 0, 10)),
	_size_archive(strtol(parsed.sizeArchive.c_str(), 0, 10)),
	_size_installed(strtol(parsed.sizeInstalled.c_str(), 0, 10)),
	_sourcepkg(parsed.sourcerpm),
	_dir_sizes(parsed.dirSizes),
#endif
      {
	_description.push_back(parsed.description);
// TODO files
      }

      YUMPackageImpl::YUMPackageImpl(
	const zypp::parser::yum::YUMPatchPackage & parsed
      )
      : _summary(parsed.summary),
	_description(),
	_buildtime(strtol(parsed.timeBuild.c_str(), 0, 10)),
	_buildhost(parsed.buildhost),
	_url(parsed.url),
	_vendor( parsed.vendor),
	_license( parsed.license),
	_packager( parsed.packager),
	_group(parsed.group),
	_changelog(), // TODO
	_type(parsed.type),
	_license_to_confirm(), // TODO add to metadata
	_authors(parsed.authors),
	_keywords( parsed.keywords),
	_mediaid(strtol(parsed.media.c_str(), 0, 10)),
	_checksum(parsed.checksumType,
		  parsed.checksum),
	_filenames(),
	_location(parsed.location),

	_install_only(parsed.installOnly)
#if 0
      : _size_package( strtol(parsed.sizePackage.c_str(), 0, 10)),
	_size_archive( strtol(parsed.sizeArchive.c_str(), 0, 10)),
	_size_installed( strtol(parsed.sizeInstalled.c_str(), 0, 10)),
	_sourcepkg( parsed.sourcerpm),
	_dir_sizes(parsed.dirSizes),
#endif
      {
	_description.push_back(parsed.description);
	for (std::list<FileData>::const_iterator it = parsed.files.begin();
	     it != parsed.files.end();
	     it++)
	{
	  _filenames.push_back(it->name);
	}
	
/*
    std::list<ChangelogEntry> changelog;
    // Package Files
    struct {
      std::string arch;
      std::string filename;
      std::string downloadsize;
      std::string md5sum;
      std::string buildtime;
    } plainRpm;
    struct {
      std::string arch;
      std::string filename;
      std::string downloadsize;
      std::string md5sum;
      std::string buildtime;
      std::list<YUMBaseVersion> baseVersions;
    } patchRpm;
    struct {
      std::string arch;
      std::string filename;
      std::string downloadsize;
      std::string md5sum;
      std::string buildtime;
      YUMBaseVersion baseVersion;
    } deltaRpm;
*/
      }


      /** Package summary */
      Label YUMPackageImpl::summary() const
      { return _summary; }

      /** Package description */
      Text YUMPackageImpl::description() const
      { return _description; }

      Text YUMPackageImpl::insnotify() const
      // metadata doesn't priovide this attribute
      { return ResObjectImplIf::insnotify(); }

      Text YUMPackageImpl::delnotify() const
      // metadata doesn't priovide this attribute
      { return ResObjectImplIf::delnotify(); }

      FSize YUMPackageImpl::size() const
      { }

      bool YUMPackageImpl::providesSources() const
      { return ResObjectImplIf::providesSources(); }

      Label YUMPackageImpl::instSrcLabel() const
      { return ResObjectImplIf::instSrcLabel(); }
      
      Vendor YUMPackageImpl::instSrcVendor() const
      { return ResObjectImplIf::instSrcVendor(); }
      
      /** */
      Date YUMPackageImpl::buildtime() const
      { return _buildtime; }

      /** */
      std::string YUMPackageImpl::buildhost() const
      { return _buildhost; }

      /** */
      Date YUMPackageImpl::installtime() const
      { return PackageImplIf::installtime(); }

      /** */
      std::string YUMPackageImpl::distribution() const
      { }

      /** */
      Vendor YUMPackageImpl::vendor() const
      { return _vendor; }

      /** */
      Label YUMPackageImpl::license() const
      { return _license; }

      /** */
      std::string YUMPackageImpl::packager() const
      { return _packager; }

      /** */
      PackageGroup YUMPackageImpl::group() const
      { return _group; }

      /** */
      Text YUMPackageImpl::changelog() const
      { return _changelog; }

      /** Don't ship it as class Url, because it might be
       * in fact anything but a legal Url. */
      std::string YUMPackageImpl::url() const
      { return _url; }

      /** */
      std::string YUMPackageImpl::os() const
      // metadata doesn't priovide this attribute
      { return PackageImplIf::os(); }

      /** */
      Text YUMPackageImpl::prein() const
      // metadata doesn't priovide this attribute
      { return PackageImplIf::prein(); }

      /** */
      Text YUMPackageImpl::postin() const
      // metadata doesn't priovide this attribute
      { return PackageImplIf::postin(); }

      /** */
      Text YUMPackageImpl::preun() const
      // metadata doesn't priovide this attribute
      { return PackageImplIf::preun(); }

      /** */
      Text YUMPackageImpl::postun() const
      // metadata doesn't priovide this attribute
      { return PackageImplIf::postun(); }

      /** */
      FSize YUMPackageImpl::sourcesize() const
      { }

      /** */
      FSize YUMPackageImpl::archivesize() const
      { }

      /** */
      Text YUMPackageImpl::authors() const
      { return _authors; }

      /** */
      Text YUMPackageImpl::filenames() const
      { return _filenames; }

      License YUMPackageImpl::licenseToConfirm() const
      { return _license_to_confirm; }

      /** */
      std::string YUMPackageImpl::type() const
      { return _type; }

      /** */
      std::list<std::string> YUMPackageImpl::keywords() const
      { return _keywords; }

      bool YUMPackageImpl::installOnly() const
      { return _install_only; }

      unsigned YUMPackageImpl::mediaId() const
      { return _mediaid; }

      PackageImplIf::CheckSum YUMPackageImpl::checksum() const
      { return _checksum; }


#if 0
      /** */
      std::list<std::string> YUMPackageImpl::insnotify() const
      { return std::list<std::string>(); }
      /** */
      std::list<std::string> YUMPackageImpl::delnotify() const
      { return std::list<std::string>(); }
      /** */
      unsigned YUMPackageImpl::packageSize() const
      { return _size_package; }
      /** */
      unsigned YUMPackageImpl::archiveSize() const
      { return _size_archive; }
      /** */
      unsigned YUMPackageImpl::installedSize() const
      { return _size_installed; }
// FIXME do not understand items below
      /** */
      bool YUMPackageImpl::providesSources() const
      {
	return false;
      }
      /** */
      std::string YUMPackageImpl::instSrcLabel() const
      {
	return "";
      }
      /** */
      std::string YUMPackageImpl::instSrcVendor() const
      {
	return "";
      }
      /** */
      unsigned YUMPackageImpl::instSrcRank() const
      {
	return 0;
      }
      /** */
      std::string YUMPackageImpl::buildhost() const
      {
	return _buildhost;
      }
      /** */
      std::string YUMPackageImpl::distribution() const
      {
	return "";
      }
      /** */
      std::string YUMPackageImpl::vendor() const
      {
	return _vendor;
      }
      /** */
      std::string YUMPackageImpl::license() const
      {
	return _license;
      }
      /** */
      std::list<std::string> YUMPackageImpl::licenseToConfirm() const
      {
	return std::list<std::string>();
      }
      /** */
      std::string YUMPackageImpl::packager() const
      {
	return _packager;
      }
      /** */
      std::string YUMPackageImpl::group() const
      {
	return _group;
      }
      /** */
      std::list<std::string> YUMPackageImpl::changelog() const
      {}
      /** */
      std::string YUMPackageImpl::url() const
      {
	return _url;
      }
      /** */
      std::string YUMPackageImpl::os() const
      {}
      /** */
      std::list<std::string> YUMPackageImpl::prein() const
      {}
      /** */
      std::list<std::string> YUMPackageImpl::postin() const
      {}
      /** */
      std::list<std::string> YUMPackageImpl::preun() const
      {}
      /** */
      std::list<std::string> YUMPackageImpl::postun() const
      {}
      /** */
      std::string YUMPackageImpl::sourcepkg() const
      { return _sourcepkg; }
      /** */
      std::list<std::string> YUMPackageImpl::authors() const
      { return _authors; }
      /** */
      std::list<std::string> YUMPackageImpl::filenames() const
      {}
      /** */
      std::list<std::string> YUMPackageImpl::recommends() const
      {}
      /** */
      std::list<std::string> YUMPackageImpl::suggests() const
      {}
      /** */
      std::string YUMPackageImpl::location() const
      {}
      /** */
      std::string YUMPackageImpl::md5sum() const
      {}
      /** */
      std::string YUMPackageImpl::externalUrl() const
      {}
      /** */
      std::list<Edition> YUMPackageImpl::patchRpmBaseVersions() const
      {}
      /** */
      unsigned YUMPackageImpl::patchRpmSize() const
      {}
      /** */
      bool YUMPackageImpl::forceInstall() const
      {}
      /** */
      std::string YUMPackageImpl::patchRpmMD5() const
      {}
      /** */
      bool YUMPackageImpl::isRemote() const
      {}
      /** */
      bool YUMPackageImpl::prefererCandidate() const
      {}

#endif

    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
