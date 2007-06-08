/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbrepository/PackageImpl.h
 *
*/

#include "zypp/TranslatedText.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/repo/RepositoryImpl.h"
#include "PackageImpl.h"


using namespace std;
using namespace zypp::detail;
using namespace::zypp::repo;

///////////////////////////////////////////////////////////////////
namespace zypp { namespace repo { namespace cached {

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : PackageImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
PackageImpl::PackageImpl (const data::RecordId &id, cached::RepoImpl::Ptr repository_r)
    : _repository (repository_r),
      _id(id)
{}

Repository
PackageImpl::repository() const
{
  return _repository->selfRepository();
}

///////////////////////////////////////////////////
// ResObject Attributes
///////////////////////////////////////////////////

TranslatedText PackageImpl::summary() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "summary" );
}

TranslatedText PackageImpl::description() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "description" );
}

TranslatedText PackageImpl::insnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "insnotify" );
}

TranslatedText PackageImpl::delnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "delnotify" );
}

TranslatedText PackageImpl::licenseToConfirm() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "licenseToConfirm" );
}

Vendor PackageImpl::vendor() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "ResObject", "vendor" );
}


ByteCount PackageImpl::size() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "size" );
}

ByteCount PackageImpl::archivesize() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "archivesize" );
}

bool PackageImpl::installOnly() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, "ResObject", "installOnly" );
}

Date PackageImpl::buildtime() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "buildtime" );
}

Date PackageImpl::installtime() const
{
  return Date();
}

unsigned PackageImpl::repositoryMediaNr() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "repositoryMediaNr" );
}

//////////////////////////////////////////
// DEPRECATED
//////////////////////////////////////////

Source_Ref PackageImpl::source() const
{
  return Source_Ref::noSource;
}

unsigned PackageImpl::sourceMediaNr() const
{
  return 1;
}

CheckSum PackageImpl::checksum() const
{
  return CheckSum();
}

std::string PackageImpl::buildhost() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "buildhost" );
}

std::string PackageImpl::distribution() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "distribution" );
}

Label PackageImpl::license() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "license" );
}

std::string PackageImpl::packager() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "packager" );
}

PackageGroup PackageImpl::group() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "group" );
}

PackageImpl::Keywords PackageImpl::keywords() const
{
  return _repository->resolvableQuery().queryStringContainerAttribute< PackageImpl::Keywords >( _id, "Package", "keywords" );
}

Changelog PackageImpl::changelog() const
{
  return Changelog();
}

Pathname PackageImpl::location() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "location" );
}

std::string PackageImpl::url() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "url" );
}

std::string PackageImpl::os() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "operatingSystem" );
}

Text PackageImpl::prein() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "prein" );
}

Text PackageImpl::postin() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "postin" );
}

Text PackageImpl::preun() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "preun" );
}

Text PackageImpl::postun() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "postun" );
}

ByteCount PackageImpl::sourcesize() const
{
  return ByteCount();
}

DiskUsage PackageImpl::diskusage() const
{
  return DiskUsage();
}

list<string> PackageImpl::authors() const
{
  return _repository->resolvableQuery().queryStringContainerAttribute< list<string> >( _id, "Package", "authors" );
}

std::list<std::string> PackageImpl::filenames() const
{
  return std::list<std::string>();
}

// std::list<DeltaRpm> PackageImpl::deltaRpms() const
// {
// return std::list<DeltaRpm>();
// }
// 
// std::list<PatchRpm> PackageImpl::patchRpms() const
// {
//   return std::list<PatchRpm>();
// }

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::cached
///////////////////////////////////////////////////////////////////

