/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbrepository/PatchImpl.h
 *
*/

#include "zypp/TranslatedText.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/repo/RepositoryImpl.h"
#include "PatchImpl.h"
#include "zypp/cache/CacheAttributes.h"


using namespace std;
using namespace zypp::detail;
using namespace::zypp::repo;

///////////////////////////////////////////////////////////////////
namespace zypp { namespace repo { namespace cached {

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : PatchImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
PatchImpl::PatchImpl (const data::RecordId &id, cached::RepoImpl::Ptr repository_r)
    : _repository (repository_r),
      _id(id)
{}

Repository
PatchImpl::repository() const
{
  return _repository->selfRepository();
}

///////////////////////////////////////////////////
// ResObject Attributes
///////////////////////////////////////////////////

TranslatedText PatchImpl::summary() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectSummary() );
}

TranslatedText PatchImpl::description() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectDescription() );
}

TranslatedText PatchImpl::insnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectInsnotify() );
}

TranslatedText PatchImpl::delnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectDelnotify() );
}

TranslatedText PatchImpl::licenseToConfirm() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, cache::attrResObjectLicenseToConfirm() );
}

Vendor PatchImpl::vendor() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrResObjectVendor() );
}


ByteCount PatchImpl::size() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, cache::attrResObjectInstalledSize() );
}

bool PatchImpl::installOnly() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, cache::attrResObjectInstallOnly() );
}

Date PatchImpl::buildtime() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, cache::attrResObjectBuildTime() );
}

Date PatchImpl::installtime() const
{
  return Date();
}

//////////////////////////////////////////
// PATCH
/////////////////////////////////////////

std::string PatchImpl::id() const
{
#warning DUBIOUS ATTRIBUTE
  return "";
  //return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPatchId() );
}

Date PatchImpl::timestamp() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, cache::attrPatchTimestamp() );
}

std::string PatchImpl::category() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, cache::attrPatchCategory() );
}

bool PatchImpl::reboot_needed() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, cache::attrPatchRebootNeeded() );
}

bool PatchImpl::affects_pkg_manager() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, cache::attrPatchAffectsPkgManager() );
}

bool PatchImpl::interactive() const
{
#warning DUBIOUS ATTRIBUTE
  return false;
  //return _repository->resolvableQuery().queryBooleanAttribute( _id, cache::attrPatchInteractive() );
}

PatchImpl::AtomList PatchImpl::all_atoms() const
{
#warning DUBIOUS ATTRIBUTE
  return PatchImpl::AtomList();
}

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::cached
///////////////////////////////////////////////////////////////////

