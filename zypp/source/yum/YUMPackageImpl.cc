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
#if 0
      : _size_package(strtol(parsed.sizePackage.c_str(), 0, 10)),
	_size_archive(strtol(parsed.sizeArchive.c_str(), 0, 10)),
	_size_installed(strtol(parsed.sizeInstalled.c_str(), 0, 10)),
	_authors(parsed.authors),
	_keywords(parsed.keywords),
	_packager(parsed.packager),
	_url(parsed.url),
	_buildhost(parsed.buildhost),
	_vendor(parsed.vendor),
	_group(parsed.group),
	_license(parsed.license),
	_type(parsed.type),
	_sourcepkg(parsed.sourcerpm),
	_checksum(parsed.checksumType,
		  string::toLower(parsed.checksumPkgid) == "yes",
		  parsed.checksum),
	_media(parsed.media),
	_time_file(strtol(parsed.timeFile.c_str(), 0, 10)),
	_time_build(strtol(parsed.timeBuild.c_str(), 0, 10)),
	_location(parsed.location),
	_files(parsed.files),
	_dir_sizes(parsed.dirSizes),
	_install_only(parsed.installOnly)
#endif
      {
      }
      YUMPackageImpl::YUMPackageImpl(
	const zypp::parser::yum::YUMPatchPackage & parsed
      )
#if 0
      : _size_package( strtol(parsed.sizePackage.c_str(), 0, 10)),
	_size_archive( strtol(parsed.sizeArchive.c_str(), 0, 10)),
	_size_installed( strtol(parsed.sizeInstalled.c_str(), 0, 10)),
	_authors( parsed.authors),
	_keywords( parsed.keywords),
	_packager( parsed.packager),
	_url( parsed.url),
	_buildhost( parsed.buildhost),
	_vendor( parsed.vendor),
	_group( parsed.group),
	_license( parsed.license),
	_type( parsed.type),
	_sourcepkg( parsed.sourcerpm),
	_checksum(parsed.checksumType,
		  string::toLower(parsed.checksumPkgid) == "yes",
		  parsed.checksum),
	_media(parsed.media),
	_time_file(strtol(parsed.timeFile.c_str(), 0, 10)),
	_time_build(strtol(parsed.timeBuild.c_str(), 0, 10)),
	_location(parsed.location),
	_files(parsed.files),
	_dir_sizes(parsed.dirSizes),
	_install_only(parsed.installOnly)
#endif
      {
	
/*
    std::string summary;
    std::string description;
    // SuSE specific data
    // Change Log
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
      { }
      /** Package description */
      Text YUMPackageImpl::description() const
      { }
      Text YUMPackageImpl::insnotify() const
      { }
      Text YUMPackageImpl::delnotify() const
      { }
      FSize YUMPackageImpl::size() const
      { }
      bool YUMPackageImpl::providesSources() const
      { }
      Label YUMPackageImpl::instSrcLabel() const
      { }
      Vendor YUMPackageImpl::instSrcVendor() const
      { }
      /** */
      Date YUMPackageImpl::buildtime() const
      { }
      /** */
      std::string YUMPackageImpl::buildhost() const
      { }
      /** */
      Date YUMPackageImpl::installtime() const
      { }
      /** */
      std::string YUMPackageImpl::distribution() const
      { }
      /** */
      Vendor YUMPackageImpl::vendor() const
      { }
      /** */
      Label YUMPackageImpl::license() const
      { }
      /** */
      std::string YUMPackageImpl::packager() const
      { }
      /** */
      PackageGroup YUMPackageImpl::group() const
      { }
      /** */
      Text YUMPackageImpl::changelog() const
      { }
      /** Don't ship it as class Url, because it might be
       * in fact anything but a legal Url. */
      std::string YUMPackageImpl::url() const
      { }
      /** */
      std::string YUMPackageImpl::os() const
      { }
      /** */
      Text YUMPackageImpl::prein() const
      { }
      /** */
      Text YUMPackageImpl::postin() const
      { }
      /** */
      Text YUMPackageImpl::preun() const
      { }
      /** */
      Text YUMPackageImpl::postun() const
      { }
      /** */
      FSize YUMPackageImpl::sourcesize() const
      { }
      /** */
      FSize YUMPackageImpl::archivesize() const
      { }
      /** */
      Text YUMPackageImpl::authors() const
      { }
      /** */
      Text YUMPackageImpl::filenames() const
      { }
      License YUMPackageImpl::licenseToConfirm() const
      { }




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
      std::string YUMPackageImpl::type() const
      { return _type; }
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
      unsigned int YUMPackageImpl::medianr() const
      {}
      /** */
      std::list<std::string> YUMPackageImpl::keywords() const
      { return _keywords; }
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
