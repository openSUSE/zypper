/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/cached/PatternImpl.cc
 *
*/
#include <iostream>

#include "zypp/base/Easy.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"

#include "zypp/CapFactory.h"
#include "zypp/TranslatedText.h"
#include "zypp/repo/RepositoryImpl.h"
#include "zypp/repo/cached/PatternImpl.h"
#include "zypp/cache/CacheAttributes.h"


using namespace std;

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


void PatternImpl::initUiCapSetFromAttr( CapSet & caps_r, const cache::Attribute & attr_r ) const
{
  std::list<std::string> capstr;
  _repository->resolvableQuery().queryStringContainerAttribute( _id, attr_r, std::back_inserter(capstr) );
  for_( it, capstr.begin(), capstr.end() )
  {
    caps_r.insert( CapFactory().parse<ResType>( *it ) );
  }
}

const CapSet & PatternImpl::includes() const
{
  if ( ! _includes )
  {
    // lazy init
    _includes.reset( new CapSet );
    initUiCapSetFromAttr( *_includes, cache::attrPatternUiIncludes() );
  }
  return *_includes;
}

const CapSet & PatternImpl::extends() const
{
  if ( ! _extends )
  {
    // lazy init
    _extends.reset( new CapSet );
    initUiCapSetFromAttr( *_extends, cache::attrPatternUiExtends() );
  }
  return *_extends;
}

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::cached
///////////////////////////////////////////////////////////////////

