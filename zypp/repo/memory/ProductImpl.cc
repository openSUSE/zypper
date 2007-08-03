/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/repo/memory/ProductImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace repo
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace memory
{ /////////////////////////////////////////////////////////////////

ProductImpl::ProductImpl( memory::RepoImpl::Ptr repo, data::Product_Ptr ptr)
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
  // TODO products attrs
{}

ProductImpl::~ProductImpl()
{}

Repository
ProductImpl::repository() const
{
  return _repository->selfRepository();
}

///////////////////////////////////////////////////
// ResObject Attributes
///////////////////////////////////////////////////

TranslatedText ProductImpl::summary() const
{
  return _summary;
}

TranslatedText ProductImpl::description() const
{
  return _description;
}

TranslatedText ProductImpl::insnotify() const
{
  return _insnotify;
}

TranslatedText ProductImpl::delnotify() const
{
  return _delnotify;
}

TranslatedText ProductImpl::licenseToConfirm() const
{
  return _license_to_confirm;
}

Vendor ProductImpl::vendor() const
{
  return _vendor;
}

ByteCount ProductImpl::size() const
{
  return _size;
}

bool ProductImpl::installOnly() const
{
  return _install_only;
}

Date ProductImpl::buildtime() const
{
  return _buildtime;
}

Date ProductImpl::installtime() const
{
  return _installtime;
}

///////////////////////////////////////////

std::string ProductImpl::type() const
{
  return _type;
}

Url ProductImpl::releaseNotesUrl() const
{
  return _release_notes_url;
}

std::list<Url> ProductImpl::updateUrls() const
{
  return _update_urls;
}

std::list<Url> ProductImpl::extraUrls() const
{
  return _extra_urls;
}

std::list<Url> ProductImpl::optionalUrls() const
{
  return _optional_urls;
}

std::list<std::string> ProductImpl::flags() const
{
  return _flags;
}

TranslatedText ProductImpl::shortName() const
{
  return TranslatedText(_shortlabel);
}

std::string ProductImpl::distributionName() const
{
  return _dist_name;
}

Edition ProductImpl::distributionEdition() const
{
  return _dist_version;
}

/////////////////////////////////////////////////////////////////
} // namespace memory
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace repository
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
