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
#include "zypp2/repo/RepositoryImpl.h"
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
    : _repository (repository_r)
    , _install_only(false)
    , _size_installed(0)
    , _size_archive(0)
    , _data_loaded(false)
    , _id(id)
{}

Repository
PackageImpl::repository() const
{
  return _repository->selfRepository();
}

/** Package summary */
TranslatedText PackageImpl::summary() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "summary" );
}

/** Package description */
TranslatedText PackageImpl::description() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "description" );
}

PackageGroup PackageImpl::group() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "group" );
}

Pathname PackageImpl::location() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Package", "group" );
}

ByteCount PackageImpl::size() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "size" );
}

/** */
ByteCount PackageImpl::archivesize() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "archivesize" );
}

bool PackageImpl::installOnly() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, "ResObject", "installOnly" );
}

unsigned PackageImpl::repositoryMediaNr() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "repositoryMediaNr" );
}

Vendor PackageImpl::vendor() const
{
  return "suse";
}

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::cached
///////////////////////////////////////////////////////////////////

