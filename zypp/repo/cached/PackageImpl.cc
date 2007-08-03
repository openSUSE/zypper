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
#include "zypp/cache/CacheAttributes.h"

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
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectSummary() );
}

TranslatedText PackageImpl::description() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectDescription() );
}

TranslatedText PackageImpl::insnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectInsnotify() );
}

TranslatedText PackageImpl::delnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectDelnotify() );
}

TranslatedText PackageImpl::licenseToConfirm() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectLicenseToConfirm() );
}

Vendor PackageImpl::vendor() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrResObjectVendor() );
}

ByteCount PackageImpl::size() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, cache::attrResObjectInstalledSize() );
}

bool PackageImpl::installOnly() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, cache::attrResObjectInstallOnly() );
}

Date PackageImpl::buildtime() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, cache::attrResObjectBuildTime() );
}

Date PackageImpl::installtime() const
{
  return Date();
}

std::string PackageImpl::buildhost() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPackageBuildhost() );
}

std::string PackageImpl::distribution() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPackageDistribution() );
}

Label PackageImpl::license() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPackageLicense() );
}

std::string PackageImpl::packager() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPackagePackager() );
}

PackageGroup PackageImpl::group() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPackageGroup() );
}

PackageImpl::Keywords PackageImpl::keywords() const
{
  PackageImpl::Keywords keywords;
  _repository->resolvableQuery().queryStringContainerAttribute( _id, cache::attrPackageKeywords(), std::inserter(keywords, keywords.begin()) );
  return keywords;
}

Changelog PackageImpl::changelog() const
{
  return Changelog();
}

unsigned PackageImpl::mediaNr() const
{
  if ( _mnr == (unsigned)-1 )
  {
    _mnr = _repository->resolvableQuery().queryNumericAttribute( _id, cache::attrPackageLocationMediaNr() );
  }
  return _mnr;
}

ByteCount PackageImpl::downloadSize() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, cache::attrPackageLocationDownloadSize() );
}

OnMediaLocation PackageImpl::location() const
{
  OnMediaLocation loc;
  queryOnMediaLocation( _repository->resolvableQuery(), _id, cache::attrPackageLocation, loc );
  return loc;
}

std::string PackageImpl::url() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPackageUrl() );
}

std::string PackageImpl::os() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPackageOperatingSystem() );
}

Text PackageImpl::prein() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPackagePrein() );
}

Text PackageImpl::postin() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPackagePostin() );
}

Text PackageImpl::preun() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPackagePreun() );
}

Text PackageImpl::postun() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPackagePostun() );
}

ByteCount PackageImpl::sourcesize() const
{
  return ByteCount();
}

const DiskUsage & PackageImpl::diskusage() const
{
  if ( ! _diskusage )
  {
    // lazy init
    _diskusage.reset( new DiskUsage );
#warning FILL DU DATA FROM DB
  }
  return *_diskusage;
}

list<string> PackageImpl::authors() const
{
  list<string> authors;
  _repository->resolvableQuery().queryStringContainerAttribute( _id, cache::attrPackageAuthors(), back_inserter(authors) );
  return authors;
}

std::list<std::string> PackageImpl::filenames() const
{
  return std::list<std::string>();
}

std::string PackageImpl::sourcePkgName() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPackageSourcePkgName() );
}

Edition PackageImpl::sourcePkgEdition() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPackageSourcePkgEdition() );
}

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::cached
///////////////////////////////////////////////////////////////////

