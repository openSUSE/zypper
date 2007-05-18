/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

//#include "zypp/source/Data/DataImpl.h"
#include "zypp2/repository/memory/DataPackageImpl.h"

using namespace std;
using zypp::data::Package_Ptr;
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace repository
{ /////////////////////////////////////////////////////////////////
namespace data
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : PackageImpl::PackageImpl
//	METHOD TYPE : Ctor
//
DataPackageImpl::DataPackageImpl( zypp::data::Package_Ptr data)
    : _data(data)
{}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : PackageImpl::~PackageImpl
//	METHOD TYPE : Dtor
//
DataPackageImpl::~DataPackageImpl()
{}

TranslatedText DataPackageImpl::summary() const
{
  return _data->summary;
}

TranslatedText DataPackageImpl::description() const
{
  return _data->description;
}

TranslatedText DataPackageImpl::insnotify() const
{
  return TranslatedText();
}

TranslatedText DataPackageImpl::delnotify() const
{
  return TranslatedText();
}

TranslatedText DataPackageImpl::licenseToConfirm() const
{
  return TranslatedText();
}

Source_Ref DataPackageImpl::source() const
{
  return Source_Ref::noSource;
}

unsigned DataPackageImpl::sourceMediaNr() const
{
  return _data->repositoryLocation.mediaNr;
}

CheckSum DataPackageImpl::checksum() const
{
  return _data->repositoryLocation.fileChecksum;
}

Date DataPackageImpl::buildtime() const
{
  return Date();
}

std::string DataPackageImpl::buildhost() const
{
  return _data->buildhost;
}

Date DataPackageImpl::installtime() const
{
  return Date();
}				// it was never installed

std::string DataPackageImpl::distribution() const
{
  return _data->distribution;
}

Vendor DataPackageImpl::vendor() const
{
  return _data->vendor;
}

Label DataPackageImpl::license() const
{
  return _data->license;
}

std::string DataPackageImpl::packager() const
{
  return std::string();
}

PackageGroup DataPackageImpl::group() const
{
  return _data->group;
}

DataPackageImpl::Keywords DataPackageImpl::keywords() const
{
  return std::set<PackageKeyword>();
}

Changelog DataPackageImpl::changelog() const
{
  return Changelog();
}

Pathname DataPackageImpl::location() const
{
  return _data->repositoryLocation.filePath;
}

std::string DataPackageImpl::url() const
{
  return _data->url;
}

std::string DataPackageImpl::os() const
{
  return _data->operatingSystem;
}

Text DataPackageImpl::prein() const
{
  return _data->prein;
}

Text DataPackageImpl::postin() const
{
  return _data->postin;
}

Text DataPackageImpl::preun() const
{
  return _data->preun;
}

Text DataPackageImpl::postun() const
{
  return _data->postun;
}

ByteCount DataPackageImpl::size() const
{
  return _data->repositoryLocation.fileSize;
}

ByteCount DataPackageImpl::sourcesize() const
// FIXME
{
  return 0;
}

ByteCount DataPackageImpl::archivesize() const
{
  return 0;
}

DiskUsage DataPackageImpl::diskusage() const
{
  return DiskUsage();
}

std::list<std::string> DataPackageImpl::authors() const
{
  return _data->authors;
}

std::list<std::string> DataPackageImpl::filenames() const
{
  return std::list<std::string>();
}

std::list<detail::PackageImplIf::DeltaRpm> DataPackageImpl::deltaRpms() const
{
  return detail::PackageImplIf::deltaRpms();
}

std::list<detail::PackageImplIf::PatchRpm> DataPackageImpl::patchRpms() const
{
  return detail::PackageImplIf::patchRpms();
}

bool DataPackageImpl::installOnly() const
{
  return false;
}

/////////////////////////////////////////////////////////////////
} // namespace Data
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
