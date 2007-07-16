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
#include "MessageImpl.h"


using namespace std;
using namespace zypp::detail;
using namespace::zypp::repo;

///////////////////////////////////////////////////////////////////
namespace zypp { namespace repo { namespace memory {

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : MessageImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
MessageImpl::MessageImpl ( memory::RepoImpl::Ptr repo, data::Message_Ptr ptr)
    : _repository(repo),

    _summary(ptr->summary),
    _description(ptr->description),
    _insnotify(ptr->insnotify),
    _delnotify(ptr->delnotify),
    _license_to_confirm(ptr->licenseToConfirm),
    _vendor(ptr->vendor),
    _size(ptr->installedSize),
    _install_only(false),
    _buildtime(ptr->buildTime),

    _text(ptr->text)
{}

Repository
MessageImpl::repository() const
{
  return _repository->selfRepository();
}

///////////////////////////////////////////////////
// ResObject Attributes
///////////////////////////////////////////////////

TranslatedText MessageImpl::summary() const
{
  return _summary;
}

TranslatedText MessageImpl::description() const
{
  return _description;
}

TranslatedText MessageImpl::insnotify() const
{
  return _insnotify;
}

TranslatedText MessageImpl::delnotify() const
{
  return _delnotify;
}

TranslatedText MessageImpl::licenseToConfirm() const
{
  return _license_to_confirm;
}

Vendor MessageImpl::vendor() const
{
  return _vendor;
}

ByteCount MessageImpl::size() const
{
  return _size;
}

bool MessageImpl::installOnly() const
{
  return _install_only;
}

Date MessageImpl::buildtime() const
{
  return _buildtime;
}

Date MessageImpl::installtime() const
{
  return _installtime;
}

//////////////////////////////////////////
// MESSAGE
/////////////////////////////////////////

TranslatedText MessageImpl::text() const
{
  return _text;
}

Patch::constPtr MessageImpl::patch() const
{
  return 0;
}
    
/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::memory
///////////////////////////////////////////////////////////////////

