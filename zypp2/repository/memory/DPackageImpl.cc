/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp2/repository/memory/DPackageImpl.cc
 *
*/

#include "zypp2/repository/memory/DPackageImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace repository
{ /////////////////////////////////////////////////////////////////
namespace memory
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : PackageImpl::PackageImpl
//	METHOD TYPE : Ctor
//
DPackageImpl::DPackageImpl(data::Package_Ptr ptr)
    : 
      _summary(ptr->summary),
      _description(ptr->description),
      _insnotify(ptr->insnotify),
      _delnotify(ptr->delnotify),
      _license_to_confirm(ptr->licenseToConfirm),
      _group(ptr->group),
      _keywords(),
      _authors(ptr->authors),
      _size(ptr->installedSize),
      _archivesize(ptr->repositoryLocation.fileSize),
      _vendor(ptr->vendor),
      _license(ptr->license),
      _buildtime(ptr->buildTime),
      _media_number(ptr->repositoryLocation.mediaNr),
      _location(ptr->repositoryLocation.filePath),
      _diskusage(),
      _checksum(ptr->repositoryLocation.fileChecksum)
{
}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : PackageImpl::~PackageImpl
//	METHOD TYPE : Dtor
//
DPackageImpl::~DPackageImpl()
{}

TranslatedText DPackageImpl::summary() const
{
  return _summary;
}

TranslatedText DPackageImpl::description() const
{
  return _description;
}

TranslatedText DPackageImpl::insnotify() const
{
  return _insnotify;
}

TranslatedText DPackageImpl::delnotify() const
{
  return _delnotify;
}

TranslatedText DPackageImpl::licenseToConfirm() const
{
  return _license_to_confirm;
}

Source_Ref DPackageImpl::source() const
{
  return Source_Ref::noSource;
}

unsigned DPackageImpl::sourceMediaNr() const
{
  return _media_number;
}

CheckSum DPackageImpl::checksum() const
{
  return _checksum;
}

Date DPackageImpl::buildtime() const
{
  return _buildtime;
}

string DPackageImpl::buildhost() const
{
  return string();
}

Date DPackageImpl::installtime() const
{
  return Date();
}				// it was never installed

string DPackageImpl::distribution() const
{
  return string();
}

Vendor DPackageImpl::vendor() const
{
  return string();
}

Label DPackageImpl::license() const
{
  return _license;
}

string DPackageImpl::packager() const
{
  return string();
}

PackageGroup DPackageImpl::group() const
{
  return _group;
}

DPackageImpl::Keywords DPackageImpl::keywords() const
{
  return _keywords;
}

Changelog DPackageImpl::changelog() const
{
  return Changelog();
}

Pathname DPackageImpl::location() const
{
  return _location;
}

string DPackageImpl::url() const
{
  return string();
}

string DPackageImpl::os() const
{
  return string();
}

Text DPackageImpl::prein() const
{
  return Text();
}

Text DPackageImpl::postin() const
{
  return Text();
}

Text DPackageImpl::preun() const
{
  return Text();
}

Text DPackageImpl::postun() const
{
  return Text();
}

ByteCount DPackageImpl::size() const
{
  return _size;
}

ByteCount DPackageImpl::sourcesize() const
// FIXME
{
  return 0;
}

ByteCount DPackageImpl::archivesize() const
{
  return _archivesize;
}

DiskUsage DPackageImpl::diskusage() const
{
  return _diskusage;
}

list<string> DPackageImpl::authors() const
{
  return list<string>();
}

list<string> DPackageImpl::filenames() const
{
  return list<string>();
}

list<detail::PackageImplIf::DeltaRpm> DPackageImpl::deltaRpms() const
{
  return detail::PackageImplIf::deltaRpms();
}

list<detail::PackageImplIf::PatchRpm> DPackageImpl::patchRpms() const
{
  return detail::PackageImplIf::patchRpms();
}

bool DPackageImpl::installOnly() const
{
  return false;
}

/////////////////////////////////////////////////////////////////
} // namespace memory
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

