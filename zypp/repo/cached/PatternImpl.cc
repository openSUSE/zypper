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
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "summary" );
}

TranslatedText PatternImpl::description() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "description" );
}

TranslatedText PatternImpl::insnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "insnotify" );
}

TranslatedText PatternImpl::delnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "delnotify" );
}

TranslatedText PatternImpl::licenseToConfirm() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "licenseToConfirm" );
}

Vendor PatternImpl::vendor() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "ResObject", "vendor" );
}


ByteCount PatternImpl::size() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "size" );
}

ByteCount PatternImpl::archivesize() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "archivesize" );
}

bool PatternImpl::installOnly() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, "ResObject", "installOnly" );
}

Date PatternImpl::buildtime() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "buildtime" );
}

Date PatternImpl::installtime() const
{
  return Date();
}

//////////////////////////////////////////
// DEPRECATED
//////////////////////////////////////////

Source_Ref PatternImpl::source() const
{
  return Source_Ref::noSource;
}

unsigned PatternImpl::mediaNr() const
{
  return 1;
}

//////////////////////////////////////////
// PATTERN
/////////////////////////////////////////

bool PatternImpl::isDefault() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, "Pattern", "isDefault" );
}

bool PatternImpl::userVisible() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, "Pattern", "userVisible" );
}

TranslatedText PatternImpl::category() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "Pattern", "isDefault" );
}

Pathname PatternImpl::icon() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Pattern", "icon" );
}

Pathname PatternImpl::script() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Pattern", "script" );
}

Label PatternImpl::order() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Pattern", "order" );
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

