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
#include "zypp2/cache/CachedResolvableDataProvider.h"
#include "zypp2/repository/RepositoryImpl.h"
#include "CachedRepositoryPackageImpl.h"


using namespace std;
using namespace zypp::detail;

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
CachedRepositoryPackageImpl::CachedRepositoryPackageImpl (Repository repository_r)
    : _repository (repository_r)
    , _install_only(false)
    , _size_installed(0)
    , _size_archive(0)
    , _data_loaded(false)
{}

Repository
CachedRepositoryPackageImpl::repository() const
{
  return _repository;
}

/** Package summary */
TranslatedText CachedRepositoryPackageImpl::summary() const
{
  return _summary;
}

/** Package description */
TranslatedText CachedRepositoryPackageImpl::description() const
{
  return _description;
}

PackageGroup CachedRepositoryPackageImpl::group() const
{
  return _group;
}

Pathname CachedRepositoryPackageImpl::location() const
{
  return _location;
}

ByteCount CachedRepositoryPackageImpl::size() const
{
  return _size_installed;
}

/** */
ByteCount CachedRepositoryPackageImpl::archivesize() const
{
  return _size_archive;
}

bool CachedRepositoryPackageImpl::installOnly() const
{
  return _install_only;
}

unsigned CachedRepositoryPackageImpl::repositoryMediaNr() const
{
  return _media_nr;
}

Vendor CachedRepositoryPackageImpl::vendor() const
{
  return "suse";
}

/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

