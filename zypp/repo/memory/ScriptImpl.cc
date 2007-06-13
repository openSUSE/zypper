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
ScriptImpl::ScriptImpl (const data::RecordId &id, memory::RepoImpl::Ptr repository_r)
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

TranslatedText ScriptImpl::summary()
{
  return _summary;
}

TranslatedText ScriptImpl::description()
{
  return _description;
}

TranslatedText ScriptImpl::insnotify()
{
  return _insnotify;
}

TranslatedText ScriptImpl::delnotify()
{
  return _delnotify;
}

TranslatedText ScriptImpl::licenseToConfirm()
{
  return _license_to_confirm;
}

Vendor ScriptImpl::vendor()
{
  return _vendor;
}

ByteCount ScriptImpl::size()
{
  return _size;
}

ByteCount ScriptImpl::archivesize()
{
  return _archivesize;
}

bool ScriptImpl::installOnly()
{
  return _install_only;
}

Date ScriptImpl::buildtime()
{
  return _buildtime;
}

Date ScriptImpl::installtime()
{
  return _installtime;
}

unsigned ScriptImpl::mediaNr()
{
  return _media_nr;
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

