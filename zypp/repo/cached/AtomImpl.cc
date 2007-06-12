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
#include "AtomImpl.h"


using namespace std;
using namespace zypp::detail;
using namespace::zypp::repo;

///////////////////////////////////////////////////////////////////
namespace zypp { namespace repo { namespace cached {

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : AtomImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
AtomImpl::AtomImpl (const data::RecordId &id, cached::RepoImpl::Ptr repository_r)
    : _repository (repository_r),
      _id(id)
{}

Repository
AtomImpl::repository() const
{
  return _repository->selfRepository();
}

///////////////////////////////////////////////////
// ResObject Attributes
///////////////////////////////////////////////////

TranslatedText AtomImpl::summary() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "summary" );
}

TranslatedText AtomImpl::description() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "description" );
}

TranslatedText AtomImpl::insnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "insnotify" );
}

TranslatedText AtomImpl::delnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "delnotify" );
}

TranslatedText AtomImpl::licenseToConfirm() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "licenseToConfirm" );
}

Vendor AtomImpl::vendor() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "ResObject", "vendor" );
}


ByteCount AtomImpl::size() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "size" );
}

ByteCount AtomImpl::archivesize() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "archivesize" );
}

bool AtomImpl::installOnly() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, "ResObject", "installOnly" );
}

Date AtomImpl::buildtime() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "buildtime" );
}

Date AtomImpl::installtime() const
{
  return Date();
}

//////////////////////////////////////////
// DEPRECATED
//////////////////////////////////////////

Source_Ref AtomImpl::source() const
{
  return Source_Ref::noSource;
}

unsigned AtomImpl::sourceMediaNr() const
{
  return 1;
}

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::cached
///////////////////////////////////////////////////////////////////

