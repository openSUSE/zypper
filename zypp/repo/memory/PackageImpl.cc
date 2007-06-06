/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/memory/PackageImpl.cc
 *
*/

#include "zypp/repo/memory/PackageImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace repo
{ /////////////////////////////////////////////////////////////////
namespace memory
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : PackageImpl::PackageImpl
//	METHOD TYPE : Ctor
//
PackageImpl::PackageImpl(data::Package_Ptr ptr)
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
PackageImpl::~PackageImpl()
{}

TranslatedText PackageImpl::summary() const
{
  return _summary;
}

TranslatedText PackageImpl::description() const
{
  return _description;
}

TranslatedText PackageImpl::insnotify() const
{
  return _insnotify;
}

TranslatedText PackageImpl::delnotify() const
{
  return _delnotify;
}

TranslatedText PackageImpl::licenseToConfirm() const
{
  return _license_to_confirm;
}

Source_Ref PackageImpl::source() const
{
  return Source_Ref::noSource;
}

unsigned PackageImpl::sourceMediaNr() const
{
  return _media_number;
}

CheckSum PackageImpl::checksum() const
{
  return _checksum;
}

Date PackageImpl::buildtime() const
{
  return _buildtime;
}

string PackageImpl::buildhost() const
{
  return string();
}

Date PackageImpl::installtime() const
{
  return Date();
}				// it was never installed

string PackageImpl::distribution() const
{
  return string();
}

Vendor PackageImpl::vendor() const
{
  return string();
}

Label PackageImpl::license() const
{
  return _license;
}

string PackageImpl::packager() const
{
  return string();
}

PackageGroup PackageImpl::group() const
{
  return _group;
}

PackageImpl::Keywords PackageImpl::keywords() const
{
  return _keywords;
}

Changelog PackageImpl::changelog() const
{
  return Changelog();
}

Pathname PackageImpl::location() const
{
  return _location;
}

string PackageImpl::url() const
{
  return string();
}

string PackageImpl::os() const
{
  return string();
}

Text PackageImpl::prein() const
{
  return Text();
}

Text PackageImpl::postin() const
{
  return Text();
}

Text PackageImpl::preun() const
{
  return Text();
}

Text PackageImpl::postun() const
{
  return Text();
}

ByteCount PackageImpl::size() const
{
  return _size;
}

ByteCount PackageImpl::sourcesize() const
// FIXME
{
  return 0;
}

ByteCount PackageImpl::archivesize() const
{
  return _archivesize;
}

DiskUsage PackageImpl::diskusage() const
{
  return _diskusage;
}

list<string> PackageImpl::authors() const
{
  return list<string>();
}

list<string> PackageImpl::filenames() const
{
  return list<string>();
}

list<detail::PackageImplIf::DeltaRpm> PackageImpl::deltaRpms() const
{
  return detail::PackageImplIf::deltaRpms();
}

list<detail::PackageImplIf::PatchRpm> PackageImpl::patchRpms() const
{
  return detail::PackageImplIf::patchRpms();
}

bool PackageImpl::installOnly() const
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

