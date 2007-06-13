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
namespace zypp { namespace repo { namespace memory {

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : AtomImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
AtomImpl::AtomImpl ( memory::RepoImpl::Ptr repo, data::Atom_Ptr ptr)
    : _repository (repo)
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
  return _summary;
}

TranslatedText AtomImpl::description() const
{
  return _description;
}

TranslatedText AtomImpl::insnotify() const
{
  return _insnotify;
}

TranslatedText AtomImpl::delnotify() const
{
  return _delnotify;
}

TranslatedText AtomImpl::licenseToConfirm() const
{
  return _license_to_confirm;
}

Vendor AtomImpl::vendor() const
{
  return _vendor;
}

ByteCount AtomImpl::size() const
{
  return _size;
}

ByteCount AtomImpl::archivesize() const
{
  return _archivesize;
}

bool AtomImpl::installOnly() const
{
  return _install_only;
}

Date AtomImpl::buildtime() const
{
  return _buildtime;
}

Date AtomImpl::installtime() const
{
  return _installtime;
}

unsigned AtomImpl::mediaNr() const
{
  return _media_nr;
}

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::memory
///////////////////////////////////////////////////////////////////

