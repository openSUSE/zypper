/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbrepository/PatternImpl.h
 *
*/

#include "zypp/TranslatedText.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/repo/RepositoryImpl.h"
#include "PatternImpl.h"
#include "zypp/cache/CacheAttributes.h"


using namespace std;
using namespace zypp::detail;
using namespace::zypp::repo;

///////////////////////////////////////////////////////////////////
namespace zypp { namespace repo { namespace cached {

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : PatternImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
PatternImpl::PatternImpl (const data::RecordId &id, cached::RepoImpl::Ptr repository_r)
    : _repository (repository_r),
      _id(id)
{}

Repository
PatternImpl::repository() const
{
  return _repository->selfRepository();
}

///////////////////////////////////////////////////
// ResObject Attributes
///////////////////////////////////////////////////

TranslatedText PatternImpl::summary() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectSummary() );
}

TranslatedText PatternImpl::description() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectDescription() );
}

TranslatedText PatternImpl::insnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectInsnotify() );
}

TranslatedText PatternImpl::delnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectDelnotify() );
}

TranslatedText PatternImpl::licenseToConfirm() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectLicenseToConfirm() );
}

Vendor PatternImpl::vendor() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrResObjectVendor() );
}


ByteCount PatternImpl::size() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, cache::attrResObjectInstalledSize() );
}

bool PatternImpl::installOnly() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, cache::attrResObjectInstallOnly() );
}

Date PatternImpl::buildtime() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, cache::attrResObjectBuildTime() );
}

Date PatternImpl::installtime() const
{
  return Date();
}

//////////////////////////////////////////
// PATTERN
/////////////////////////////////////////

bool PatternImpl::isDefault() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, cache::attrPatternIsDefault() );
}

bool PatternImpl::userVisible() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, cache::attrPatternUserVisible() );
}

TranslatedText PatternImpl::category() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrPatternCategory() );
}

Pathname PatternImpl::icon() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPatternIcon() );
}

Pathname PatternImpl::script() const
{
#warning DUBIOUS ATTRIBUTE
  return "";
  //return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPatternScript() );
}

Label PatternImpl::order() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPatternOrder() );
}

//std::set<std::string> install_packages( const Locale & lang = Locale("") ) const;
// const CapSet & PatternImpl::includes() const
// {
//
// }
//
// const CapSet & PatternImpl::extends() const
// {
//
// }

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::cached
///////////////////////////////////////////////////////////////////

