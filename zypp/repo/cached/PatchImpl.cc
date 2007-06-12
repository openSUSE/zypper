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
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "summary" );
}

TranslatedText PatchImpl::description() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "description" );
}

TranslatedText PatchImpl::insnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "insnotify" );
}

TranslatedText PatchImpl::delnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "delnotify" );
}

TranslatedText PatchImpl::licenseToConfirm() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "licenseToConfirm" );
}

Vendor PatchImpl::vendor() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "ResObject", "vendor" );
}


ByteCount PatchImpl::size() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "size" );
}

ByteCount PatchImpl::archivesize() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "archivesize" );
}

bool PatchImpl::installOnly() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, "ResObject", "installOnly" );
}

Date PatchImpl::buildtime() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "buildtime" );
}

Date PatchImpl::installtime() const
{
  return Date();
}

//////////////////////////////////////////
// DEPRECATED
//////////////////////////////////////////

Source_Ref PatchImpl::source() const
{
  return Source_Ref::noSource;
}

unsigned PatchImpl::mediaNr() const
{
  return 1;
}

//////////////////////////////////////////
// PATCH
/////////////////////////////////////////

std::string PatchImpl::id() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Patch", "id" );
}

Date PatchImpl::timestamp() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "Patch", "timestamp" );
}

std::string PatchImpl::category() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Patch", "category" );
}

bool PatchImpl::reboot_needed() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, "Patch", "rebootNeeded" );
}

bool PatchImpl::affects_pkg_manager() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, "Patch", "affectsPkgManager" );
}

bool PatchImpl::interactive() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, "Patch", "interactive" );
}

PatchImpl::AtomList PatchImpl::all_atoms() const
{
  return PatchImpl::AtomList();
}

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::cached
///////////////////////////////////////////////////////////////////

