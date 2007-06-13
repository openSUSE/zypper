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
AtomImpl::AtomImpl (const data::RecordId &id, memory::RepoImpl::Ptr repository_r)
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

TranslatedText AtomImpl::summary()
{
  return _summary;
}

TranslatedText AtomImpl::description()
{
  return _description;
}

TranslatedText AtomImpl::insnotify()
{
  return _insnotify;
}

TranslatedText AtomImpl::delnotify()
{
  return _delnotify;
}

TranslatedText AtomImpl::licenseToConfirm()
{
  return _license_to_confirm;
}

Vendor AtomImpl::vendor()
{
  return _vendor;
}

ByteCount AtomImpl::size()
{
  return _size;
}

ByteCount AtomImpl::archivesize()
{
  return _archivesize;
}

bool AtomImpl::installOnly()
{
  return _install_only;
}

Date AtomImpl::buildtime()
{
  return _buildtime;
}

Date AtomImpl::installtime()
{
  return _installtime;
}

unsigned AtomImpl::mediaNr()
{
  return _media_nr;
}

//////////////////////////////////////////
// DEPRECATED
//////////////////////////////////////////

Source_Ref AtomImpl::source() const
{
  return Source_Ref::noSource;
}

unsigned AtomImpl::mediaNr() const
{
  return 1;
}

/////////////////////////////////////////////////////////////////
} } } // namespace zypp::repo::memory
///////////////////////////////////////////////////////////////////

