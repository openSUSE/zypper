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
#include "ScriptImpl.h"
#include "zypp/cache/CacheAttributes.h"


using namespace std;
using namespace zypp::detail;
using namespace::zypp::repo;

///////////////////////////////////////////////////////////////////
namespace zypp { namespace repo { namespace cached {

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : ScriptImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
ScriptImpl::ScriptImpl (const data::RecordId &id, cached::RepoImpl::Ptr repository_r)
    : _repository (repository_r),
      _id(id)
{}

Repository
ScriptImpl::repository() const
{
  return _repository->selfRepository();
}

///////////////////////////////////////////////////
// ResObject Attributes
///////////////////////////////////////////////////

TranslatedText ScriptImpl::summary() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectSummary() );
}

TranslatedText ScriptImpl::description() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectDescription() );
}

TranslatedText ScriptImpl::insnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectInsnotify() );
}

TranslatedText ScriptImpl::delnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectDelnotify() );
}

TranslatedText ScriptImpl::licenseToConfirm() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectLicenseToConfirm() );
}

Vendor ScriptImpl::vendor() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrResObjectVendor() );
}


ByteCount ScriptImpl::size() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, cache::attrResObjectInstalledSize() );
}

bool ScriptImpl::installOnly() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, cache::attrResObjectInstallOnly() );
}

Date ScriptImpl::buildtime() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, cache::attrResObjectBuildTime() );
}

Date ScriptImpl::installtime() const
{
  return Date();
}

//////////////////////////////////////////
// SCRIPT
/////////////////////////////////////////

std::string ScriptImpl::doScriptInlined() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrScriptDoScript() );
}

OnMediaLocation ScriptImpl::doScriptLocation() const
{
  OnMediaLocation loc;
  queryOnMediaLocation( _repository->resolvableQuery(), _id, cache::attrScriptDoScriptLocation, loc );
  return loc;
}

std::string ScriptImpl::undoScriptInlined() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrScriptUndoScript() );
}

OnMediaLocation ScriptImpl::undoScriptLocation() const
{
  OnMediaLocation loc;
  queryOnMediaLocation( _repository->resolvableQuery(), _id, cache::attrScriptUndoScriptLocation, loc );
  return loc;
}

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::cached
///////////////////////////////////////////////////////////////////

