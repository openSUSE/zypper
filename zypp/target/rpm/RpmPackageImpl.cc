/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/RpmPackageImpl.cc
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
    _installtime(data->tag_installtime()),
    _buildhost(data->tag_buildhost()),
    _url(data->tag_url()),
    _vendor(data->tag_vendor()),
    _license(data->tag_license()),
    _packager(data->tag_packager()),
    _group(data->tag_group()),
    _changelog(data->tag_changelog()),
    _type("rpm"), // FIXME in the future
    _filenames(data->tag_filenames()),
//	_disk_usage(data->diskusage),
    _size(data->tag_size())
{
  // we know we are reading english.
  _description.setText(data->tag_description(), Locale("en"));
  data->tag_du(_disk_usage);
  _location.setDownloadSize(data->tag_archivesize());
}

/** Package summary */
TranslatedText RPMPackageImpl::summary() const
{
  return _summary;
}

/** Package description */
TranslatedText RPMPackageImpl::description() const
{
  return _description;
}

ByteCount RPMPackageImpl::size() const
{
  return _size;
}

/** */
Date RPMPackageImpl::buildtime() const
{
  return _buildtime;
}

/** */
std::string RPMPackageImpl::buildhost() const
{
  return _buildhost;
}

/** */
Date RPMPackageImpl::installtime() const
{
  return _installtime;
}

/** */
std::string RPMPackageImpl::distribution() const
#warning fixme
{
  return string();
}

/** */
Vendor RPMPackageImpl::vendor() const
{
  return _vendor;
}

/** */
Label RPMPackageImpl::license() const
{
  return _license;
}

/** */
std::string RPMPackageImpl::packager() const
{
  return _packager;
}

/** */
PackageGroup RPMPackageImpl::group() const
{
  return _group;
}

/** */
Changelog RPMPackageImpl::changelog() const
{
  return _changelog;
}

/** */
OnMediaLocation RPMPackageImpl::location() const
{
  return _location;
}

/** Don't ship it as class Url, because it might be
 * in fact anything but a legal Url. */
std::string RPMPackageImpl::url() const
{
  return _url;
}

/** */
std::string RPMPackageImpl::os() const
// metadata doesn't priovide this attribute
{
  return PackageImplIf::os();
}

/** */
Text RPMPackageImpl::prein() const
// metadata doesn't priovide this attribute
{
  return PackageImplIf::prein();
}

/** */
Text RPMPackageImpl::postin() const
// metadata doesn't priovide this attribute
{
  return PackageImplIf::postin();
}

/** */
Text RPMPackageImpl::preun() const
// metadata doesn't priovide this attribute
{
  return PackageImplIf::preun();
}

/** */
Text RPMPackageImpl::postun() const
// metadata doesn't priovide this attribute
{
  return PackageImplIf::postun();
}

/** */
ByteCount RPMPackageImpl::sourcesize() const
#warning fixme
{
  return 0;
}

/** */
std::list<std::string> RPMPackageImpl::filenames() const
{
  return _filenames;
}

/** */
std::string RPMPackageImpl::type() const
{
  return _type;
}

/** */
DiskUsage RPMPackageImpl::diskUsage() const
{
  return _disk_usage;
}

/** */
Repository RPMPackageImpl::repository() const
{
  return _repository;
}

} // namespace rpm
/////////////////////////////////////////////////////////////////
} // namespace target
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
