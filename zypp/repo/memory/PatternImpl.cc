/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/repo/memory/PatternImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace repo
{ /////////////////////////////////////////////////////////////////
namespace memory
{

PatternImpl::PatternImpl( repo::memory::RepoImpl::Ptr repo, data::Pattern_Ptr ptr)
  : _repository(repo),
  _summary(ptr->summary),
  _description(ptr->description),
  _insnotify(ptr->insnotify),
  _delnotify(ptr->delnotify),
  _license_to_confirm(ptr->licenseToConfirm),
  _vendor(ptr->vendor),
  _size(ptr->installedSize),
  _install_only(false),
  _buildtime(ptr->buildTime)
  //TODO pattern attrs
{

}

PatternImpl::~PatternImpl()
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
  return _summary;
}

TranslatedText PatternImpl::description() const
{
  return _description;
}

TranslatedText PatternImpl::insnotify() const
{
  return _insnotify;
}

TranslatedText PatternImpl::delnotify() const
{
  return _delnotify;
}

TranslatedText PatternImpl::licenseToConfirm() const
{
  return _license_to_confirm;
}

Vendor PatternImpl::vendor() const
{
  return _vendor;
}

ByteCount PatternImpl::size() const
{
  return _size;
}

bool PatternImpl::installOnly() const
{
  return _install_only;
}

Date PatternImpl::buildtime() const
{
  return _buildtime;
}

Date PatternImpl::installtime() const
{
  return _installtime;
}

///////////////////////////////////////

TranslatedText PatternImpl::category() const
{
  return _category;
}

bool PatternImpl::userVisible() const
{
  return _visible;
}

Label PatternImpl::order() const
{
  return _order;
}

Pathname PatternImpl::icon() const
{
  return _icon;
}

/////////////////////////////////////////////////////////////////
} // namespace detail
///////////////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
