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

TranslatedText PatchImpl::summary()
{
  return _summary;
}

TranslatedText PatchImpl::description()
{
  return _description;
}

TranslatedText PatchImpl::insnotify()
{
  return _insnotify;
}

TranslatedText PatchImpl::delnotify()
{
  return _delnotify;
}

TranslatedText PatchImpl::licenseToConfirm()
{
  return _license_to_confirm;
}

Vendor PatchImpl::vendor()
{
  return _vendor;
}

ByteCount PatchImpl::size()
{
  return _size;
}

ByteCount PatchImpl::archivesize()
{
  return _archivesize;
}

bool PatchImpl::installOnly()
{
  return _install_only;
}

Date PatchImpl::buildtime()
{
  return _buildtime;
}

Date PatchImpl::installtime()
{
  return _installtime;
}

unsigned PatchImpl::mediaNr()
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

