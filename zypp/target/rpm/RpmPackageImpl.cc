/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/RPMPackageImpl.cc
 *
*/

#include "zypp/target/rpm/RpmPackageImpl.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"

#include <list>
#include <string>

using namespace std;
using namespace zypp::detail;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////
    namespace rpm
    {
      ///////////////////////////////////////////////////////////////////
      //
      //        CLASS NAME : RPMPackageImpl
      //
      ///////////////////////////////////////////////////////////////////


      RPMPackageImpl::RPMPackageImpl(
	const RpmHeader::constPtr data
      )
        : _summary(data->tag_summary(), Locale("en")),
	_description(),
	_buildtime(data->tag_buildtime()),
	_buildhost(data->tag_buildhost()),
	_url(data->tag_url()),
	_vendor(data->tag_vendor()),
	_license(data->tag_license()),
	_packager(data->tag_packager()),
	_group(data->tag_group()),
	_changelog(data->tag_changelog()),
	_type("rpm"), // FIXME in the future
//	_authors(data->authors),
//	_keywords(data->keywords),
	_filenames(data->tag_filenames()),
//	_disk_usage(data->diskusage),
	_size(data->tag_size()),
	_archivesize(data->tag_archivesize())
#if 0
	_size_installed( strtol(parsed.sizeInstalled.c_str(), 0, 10)),
	_sourcepkg( parsed.sourcerpm),
	_dir_sizes(parsed.dirSizes),
#endif
      {
        // we know we are reading english.
        _description.setText(data->tag_description(), Locale("en"));
	data->tag_du(_disk_usage);
      }

      /** Package summary */
      TranslatedText RPMPackageImpl::summary() const
      { return _summary; }

      /** Package description */
      TranslatedText RPMPackageImpl::description() const
      { return _description; }

      Text RPMPackageImpl::insnotify() const
      // metadata doesn't priovide this attribute
      { return ResObjectImplIf::insnotify(); }

      Text RPMPackageImpl::delnotify() const
      // metadata doesn't priovide this attribute
      { return ResObjectImplIf::delnotify(); }

      ByteCount RPMPackageImpl::size() const
      { return _size; }

      bool RPMPackageImpl::providesSources() const
      { return ResObjectImplIf::providesSources(); }

      Label RPMPackageImpl::instSrcLabel() const
      { return ResObjectImplIf::instSrcLabel(); }
      
      Vendor RPMPackageImpl::instSrcVendor() const
      { return ResObjectImplIf::instSrcVendor(); }
      
      /** */
      Date RPMPackageImpl::buildtime() const
      { return _buildtime; }

      /** */
      std::string RPMPackageImpl::buildhost() const
      { return _buildhost; }

      /** */
      Date RPMPackageImpl::installtime() const
      { return PackageImplIf::installtime(); }

      /** */
      std::string RPMPackageImpl::distribution() const
#warning fixme
      { return string(); }

      /** */
      Vendor RPMPackageImpl::vendor() const
      { return _vendor; }

      /** */
      Label RPMPackageImpl::license() const
      { return _license; }

      /** */
      std::string RPMPackageImpl::packager() const
      { return _packager; }

      /** */
      PackageGroup RPMPackageImpl::group() const
      { return _group; }

      /** */
      Changelog RPMPackageImpl::changelog() const
      { return _changelog; }

      /** */
      Pathname RPMPackageImpl::location() const
      { return _location; }

      /** Don't ship it as class Url, because it might be
       * in fact anything but a legal Url. */
      std::string RPMPackageImpl::url() const
      { return _url; }

      /** */
      std::string RPMPackageImpl::os() const
      // metadata doesn't priovide this attribute
      { return PackageImplIf::os(); }

      /** */
      Text RPMPackageImpl::prein() const
      // metadata doesn't priovide this attribute
      { return PackageImplIf::prein(); }

      /** */
      Text RPMPackageImpl::postin() const
      // metadata doesn't priovide this attribute
      { return PackageImplIf::postin(); }

      /** */
      Text RPMPackageImpl::preun() const
      // metadata doesn't priovide this attribute
      { return PackageImplIf::preun(); }

      /** */
      Text RPMPackageImpl::postun() const
      // metadata doesn't priovide this attribute
      { return PackageImplIf::postun(); }

      /** */
      ByteCount RPMPackageImpl::sourcesize() const
#warning fixme
      { return 0; }

      /** */
      ByteCount RPMPackageImpl::archivesize() const
      { return _archivesize; }

      /** */
      std::list<std::string> RPMPackageImpl::authors() const
      { return _authors; }

      /** */
      std::list<std::string> RPMPackageImpl::filenames() const
      { return _filenames; }

      License RPMPackageImpl::licenseToConfirm() const
      { return _license_to_confirm; }

      /** */
      std::string RPMPackageImpl::type() const
      { return _type; }

      /** */
      std::list<std::string> RPMPackageImpl::keywords() const
      { return _keywords; }

      /** */
      DiskUsage RPMPackageImpl::diskUsage() const
      { return _disk_usage; }

      /** */
      Source_Ref RPMPackageImpl::source() const
      { return _source; }
#if 0
      /** */
      std::list<std::string> RPMPackageImpl::insnotify() const
      { return std::list<std::string>(); }
      /** */
      std::list<std::string> RPMPackageImpl::delnotify() const
      { return std::list<std::string>(); }
      /** */
      unsigned RPMPackageImpl::packageSize() const
      { return _size_package; }
      /** */
      unsigned RPMPackageImpl::archiveSize() const
      { return _size_archive; }
      /** */
      unsigned RPMPackageImpl::installedSize() const
      { return _size_installed; }
// FIXME do not understand items below
      /** */
      bool RPMPackageImpl::providesSources() const
      {
	return false;
      }
      /** */
      std::string RPMPackageImpl::instSrcLabel() const
      {
	return "";
      }
      /** */
      std::string RPMPackageImpl::instSrcVendor() const
      {
	return "";
      }
      /** */
      unsigned RPMPackageImpl::instSrcRank() const
      {
	return 0;
      }
      /** */
      std::string RPMPackageImpl::buildhost() const
      {
	return _buildhost;
      }
      /** */
      std::string RPMPackageImpl::distribution() const
      {
	return "";
      }
      /** */
      std::string RPMPackageImpl::vendor() const
      {
	return _vendor;
      }
      /** */
      std::string RPMPackageImpl::license() const
      {
	return _license;
      }
      /** */
      std::list<std::string> RPMPackageImpl::licenseToConfirm() const
      {
	return std::list<std::string>();
      }
      /** */
      std::string RPMPackageImpl::packager() const
      {
	return _packager;
      }
      /** */
      std::string RPMPackageImpl::group() const
      {
	return _group;
      }
      /** */
      std::list<std::string> RPMPackageImpl::changelog() const
      {}
      /** */
      std::string RPMPackageImpl::url() const
      {
	return _url;
      }
      /** */
      std::string RPMPackageImpl::os() const
      {}
      /** */
      std::list<std::string> RPMPackageImpl::prein() const
      {}
      /** */
      std::list<std::string> RPMPackageImpl::postin() const
      {}
      /** */
      std::list<std::string> RPMPackageImpl::preun() const
      {}
      /** */
      std::list<std::string> RPMPackageImpl::postun() const
      {}
      /** */
      std::string RPMPackageImpl::sourcepkg() const
      { return _sourcepkg; }
      /** */
      std::list<std::string> RPMPackageImpl::authors() const
      { return _authors; }
      /** */
      std::list<std::string> RPMPackageImpl::filenames() const
      {}
      /** */
      std::list<std::string> RPMPackageImpl::recommends() const
      {}
      /** */
      std::list<std::string> RPMPackageImpl::suggests() const
      {}
      /** */
      std::string RPMPackageImpl::location() const
      {}
      /** */
      std::string RPMPackageImpl::md5sum() const
      {}
      /** */
      std::string RPMPackageImpl::externalUrl() const
      {}
      /** */
      std::list<Edition> RPMPackageImpl::patchRpmBaseVersions() const
      {}
      /** */
      unsigned RPMPackageImpl::patchRpmSize() const
      {}
      /** */
      bool RPMPackageImpl::forceInstall() const
      {}
      /** */
      std::string RPMPackageImpl::patchRpmMD5() const
      {}
      /** */
      bool RPMPackageImpl::isRemote() const
      {}
      /** */
      bool RPMPackageImpl::prefererCandidate() const
      {}

#endif

    } // namespace rpm
    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
