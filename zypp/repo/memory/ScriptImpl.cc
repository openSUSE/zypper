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
namespace zypp { namespace repo { namespace memory {

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : ScriptImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
ScriptImpl::ScriptImpl ( memory::RepoImpl::Ptr repo, data::Script_Ptr ptr)
    : _repository (repo),
    _summary(ptr->summary),
    _description(ptr->description),
    _insnotify(ptr->insnotify),
    _delnotify(ptr->delnotify),
    _license_to_confirm(ptr->licenseToConfirm),
    _vendor(ptr->vendor),
    _size(ptr->installedSize),
    _install_only(false),
    _buildtime(ptr->buildTime)
    //TODO script attrs
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
  return _summary;
}

TranslatedText ScriptImpl::description() const
{
  return _description;
}

TranslatedText ScriptImpl::insnotify() const
{
  return _insnotify;
}

TranslatedText ScriptImpl::delnotify() const
{
  return _delnotify;
}

TranslatedText ScriptImpl::licenseToConfirm() const
{
  return _license_to_confirm;
}

Vendor ScriptImpl::vendor() const
{
  return _vendor;
}

ByteCount ScriptImpl::size() const
{
  return _size;
}

bool ScriptImpl::installOnly() const
{
  return _install_only;
}

Date ScriptImpl::buildtime() const
{
  return _buildtime;
}

Date ScriptImpl::installtime() const
{
  return _installtime;
}

//////////////////////////////////////////
// MESSAGE
/////////////////////////////////////////

Pathname ScriptImpl::do_script() const
{
  return Pathname();
}

Pathname ScriptImpl::undo_script() const
{
  return Pathname();
}

bool ScriptImpl::undo_available() const
{
  return false;
}
    

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::memory
///////////////////////////////////////////////////////////////////

