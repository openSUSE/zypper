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

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : PatchImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
PatchImpl::PatchImpl ( repo::memory::RepoImpl::Ptr repo, data::Patch_Ptr ptr)
  : _repository(repo)
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

ByteCount PatchImpl::archivesize() const
{
  return _archivesize;
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

unsigned PatchImpl::mediaNr() const
{
  return _media_nr;
}


//////////////////////////////////////////
// PATCH
/////////////////////////////////////////

std::string PatchImpl::id() const
{
  return _repository->resolvableQuery().queryStringAttribute( _id, "Patch", "id" );
}

Date PatchImpl::timestamp() const
{
  return _patch_id;
}

std::string PatchImpl::category() const
{
  return _category;
}

bool PatchImpl::reboot_needed() const
{
  return _reboot_nedeed;
}

bool PatchImpl::affects_pkg_manager() const
{
  return _affects_pkg_manager;
}

PatchImpl::AtomList PatchImpl::all_atoms() const
{
  return _atoms;
}

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::memory
///////////////////////////////////////////////////////////////////

