/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbrepository/CachedRepositoryPackageImpl.h
 *
*/

#include "zypp/TranslatedText.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp2/repository/RepositoryImpl.h"
#include "CachedRepositoryPackageImpl.h"


using namespace std;
using namespace zypp::detail;
using zypp::repository::cached::CachedRepositoryImpl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : CachedRepositoryPackageImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
CachedRepositoryPackageImpl::CachedRepositoryPackageImpl (const data::RecordId &id, repository::cached::CachedRepositoryImpl::Ptr repository_r)
    : _repository (repository_r)
    , _install_only(false)
    , _size_installed(0)
    , _size_archive(0)
    , _data_loaded(false)
    , _id(id)
{}

Repository
CachedRepositoryPackageImpl::repository() const
{
  return _repository->selfRepository();
}

/** Package summary */
TranslatedText CachedRepositoryPackageImpl::summary() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "summary" );
}

/** Package description */
TranslatedText CachedRepositoryPackageImpl::description() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "description" );
}

PackageGroup CachedRepositoryPackageImpl::group() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "group" );
}

Pathname CachedRepositoryPackageImpl::location() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "group" );
}

ByteCount CachedRepositoryPackageImpl::size() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "size" );
}

/** */
ByteCount CachedRepositoryPackageImpl::archivesize() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "archivesize" );
}

bool CachedRepositoryPackageImpl::installOnly() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "installOnly" );
}

unsigned CachedRepositoryPackageImpl::repositoryMediaNr() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "repositoryMediaNr" );
}

Vendor CachedRepositoryPackageImpl::vendor() const
{
  return "suse";
}

/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

