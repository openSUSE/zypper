/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/TranslatedText.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/repo/RepositoryImpl.h"
#include "ProductImpl.h"
#include "zypp/cache/CacheAttributes.h"


using namespace std;
using namespace zypp::detail;
using namespace::zypp::repo;

///////////////////////////////////////////////////////////////////
namespace zypp { namespace repo { namespace cached {

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : ProductImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
ProductImpl::ProductImpl (const data::RecordId &id, cached::RepoImpl::Ptr repository_r)
    : _repository (repository_r),
      _id(id)
{}

Repository
ProductImpl::repository() const
{
  return _repository->selfRepository();
}

///////////////////////////////////////////////////
// ResObject Attributes
///////////////////////////////////////////////////

TranslatedText ProductImpl::summary() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectSummary() );
}

TranslatedText ProductImpl::description() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectDescription() );
}

TranslatedText ProductImpl::insnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectInsnotify() );
}

TranslatedText ProductImpl::delnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectDelnotify() );
}

TranslatedText ProductImpl::licenseToConfirm() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectLicenseToConfirm() );
}

Vendor ProductImpl::vendor() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrResObjectVendor() );
}


ByteCount ProductImpl::size() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, cache::attrResObjectInstalledSize() );
}

bool ProductImpl::installOnly() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, cache::attrResObjectInstallOnly() );
}

Date ProductImpl::buildtime() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, cache::attrResObjectBuildTime() );
}

Date ProductImpl::installtime() const
{
  return Date();
}

//////////////////////////////////////////
// PRODUCT
/////////////////////////////////////////

std::string ProductImpl::type() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrProductType() );
}

Url ProductImpl::releaseNotesUrl() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrProductReleasenotesUrl() );
}

std::list<Url> ProductImpl::updateUrls() const
{
  std::list<Url> urls;
  _repository->resolvableQuery().queryStringContainerAttribute( _id, cache::attrProductUpdateUrls(), back_inserter(urls) );
  return urls;
}

std::list<Url> ProductImpl::extraUrls() const
{
  std::list<Url> urls;
  _repository->resolvableQuery().queryStringContainerAttribute( _id, cache::attrProductExtraUrls(), back_inserter(urls) );
  return urls;
}

std::list<Url> ProductImpl::optionalUrls() const
{
  std::list<Url> urls;
  _repository->resolvableQuery().queryStringContainerAttribute( _id, cache::attrProductOptionalUrls(), back_inserter(urls) );
  return urls;
}

list<string> ProductImpl::flags() const
{
  list<string> flags;
  _repository->resolvableQuery().queryStringContainerAttribute( _id, cache::attrProductFlags(), back_inserter(flags) );
  return flags;
}

TranslatedText ProductImpl::shortName() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrProductShortName() );
}

std::string ProductImpl::distributionName() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrProductDistributionName() );
}

Edition ProductImpl::distributionEdition() const
{
  return Edition( _repository->resolvableQuery().queryStringAttribute( _id, cache::attrProductDistributionEdition() ) );
}

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::cached
///////////////////////////////////////////////////////////////////

