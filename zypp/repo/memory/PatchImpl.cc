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
#include "PatchImpl.h"

using namespace std;
using namespace zypp::detail;
using namespace::zypp::repo;

///////////////////////////////////////////////////////////////////
namespace zypp { namespace repo { namespace memory {

PatchImpl::PatchImpl ( repo::memory::RepoImpl::Ptr repo, data::Patch_Ptr ptr)
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

  _patch_id(ptr->id),
  _timestamp(ptr->timestamp),
  _category(ptr->category),
  _reboot_needed(ptr->rebootNeeded),
  _affects_pkg_manager(ptr->affectsPkgManager)
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
  return _summary;
}

TranslatedText PatchImpl::description() const
{
  return _description;
}

TranslatedText PatchImpl::insnotify() const
{
  return _insnotify;
}

TranslatedText PatchImpl::delnotify() const
{
  return _delnotify;
}

TranslatedText PatchImpl::licenseToConfirm() const
{
  return _license_to_confirm;
}

Vendor PatchImpl::vendor() const
{
  return _vendor;
}

ByteCount PatchImpl::size() const
{
  return _size;
}

bool PatchImpl::installOnly() const
{
  return _install_only;
}

Date PatchImpl::buildtime() const
{
  return _buildtime;
}

Date PatchImpl::installtime() const
{
  return _installtime;
}

//////////////////////////////////////////
// PATCH
/////////////////////////////////////////

std::string PatchImpl::id() const
{
  return _patch_id;
}

Date PatchImpl::timestamp() const
{
  return _timestamp;
}

std::string PatchImpl::category() const
{
  return _category;
}

bool PatchImpl::reboot_needed() const
{
  return _reboot_needed;
}

bool PatchImpl::affects_pkg_manager() const
{
  return _affects_pkg_manager;
}

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::memory
///////////////////////////////////////////////////////////////////

