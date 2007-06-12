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
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "summary" );
}

TranslatedText ScriptImpl::description() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "description" );
}

TranslatedText ScriptImpl::insnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "insnotify" );
}

TranslatedText ScriptImpl::delnotify() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "delnotify" );
}

TranslatedText ScriptImpl::licenseToConfirm() const
{
  return _repository->resolvableQuery().queryTranslatedStringAttribute( _id, "ResObject", "licenseToConfirm" );
}

Vendor ScriptImpl::vendor() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "ResObject", "vendor" );
}


ByteCount ScriptImpl::size() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "size" );
}

ByteCount ScriptImpl::archivesize() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "archivesize" );
}

bool ScriptImpl::installOnly() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, "ResObject", "installOnly" );
}

Date ScriptImpl::buildtime() const
{
  return _repository->resolvableQuery().queryNumericAttribute( _id, "ResObject", "buildtime" );
}

Date ScriptImpl::installtime() const
{
  return Date();
}

//////////////////////////////////////////
// DEPRECATED
//////////////////////////////////////////

Source_Ref ScriptImpl::source() const
{
  return Source_Ref::noSource;
}

unsigned ScriptImpl::sourceMediaNr() const
{
  return 1;
}

//////////////////////////////////////////
// MESSAGE
/////////////////////////////////////////

Pathname ScriptImpl::do_script() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Script", "doScript" );
}

Pathname ScriptImpl::undo_script() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Script", "undoScript" );
}

bool ScriptImpl::undo_available() const
{
  return _repository->resolvableQuery().queryBooleanAttribute( _id, "Script", "undoAvailable", false );
}
    

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::cached
///////////////////////////////////////////////////////////////////

