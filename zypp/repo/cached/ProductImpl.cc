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
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "summary" );
}

TranslatedText ProductImpl::description() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "description" );
}

TranslatedText ProductImpl::insnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "insnotify" );
}

TranslatedText ProductImpl::delnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "delnotify" );
}

TranslatedText ProductImpl::licenseToConfirm() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "licenseToConfirm" );
}

Vendor ProductImpl::vendor() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "ResObject", "vendor" );
}


ByteCount ProductImpl::size() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "size" );
}

ByteCount ProductImpl::archivesize() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "archivesize" );
}

bool ProductImpl::installOnly() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, "ResObject", "installOnly" );
}

Date ProductImpl::buildtime() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "buildtime" );
}

Date ProductImpl::installtime() const
{
  return Date();
}

//////////////////////////////////////////
// DEPRECATED
//////////////////////////////////////////

Source_Ref ProductImpl::source() const
{
  return Source_Ref::noSource;
}

unsigned ProductImpl::mediaNr() const
{
  return 1;
}

//////////////////////////////////////////
// PRODUCT
/////////////////////////////////////////

std::string ProductImpl::category() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Product", "category" );
}

Url ProductImpl::releaseNotesUrl() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Product", "releaseNotesUrl" );
}

std::list<Url> ProductImpl::updateUrls() const
{
  std::list<Url> urls;
  _repository->resolvableQuery().queryStringContainerAttribute( _id, "Product", "updateUrls", back_inserter(urls) );
  return urls;
}

std::list<Url> ProductImpl::extraUrls() const
{
  std::list<Url> urls;
  _repository->resolvableQuery().queryStringContainerAttribute( _id, "Product", "extraUrls", back_inserter(urls) );
  return urls;
}

std::list<Url> ProductImpl::optionalUrls() const
{
  std::list<Url> urls;
  _repository->resolvableQuery().queryStringContainerAttribute( _id, "Product", "optionalUrls", back_inserter(urls) );
  return urls;
}

list<string> ProductImpl::flags() const
{
  list<string> flags;
  _repository->resolvableQuery().queryStringContainerAttribute( _id, "Product", "flags", back_inserter(flags) );
  return flags;
}

TranslatedText ProductImpl::shortName() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "Product", "shortName" );
}

std::string ProductImpl::distributionName() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Product", "distributionName" );
}

Edition ProductImpl::distributionEdition() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Product", "distributionEdition" );
}

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::cached
///////////////////////////////////////////////////////////////////

