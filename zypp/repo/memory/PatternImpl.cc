/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repository/memory/PatternImpl.cc
 *
*/
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
///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : PatternImpl::PatternImpl
//	METHOD TYPE : Ctor
//
PatternImpl::PatternImpl( repo::memory::RepoImpl::Ptr repo, data::Pattern_Ptr ptr)
  : _repository(repo)
{

}

PatternImpl::~PatternImpl()
{}

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

ByteCount PatternImpl::archivesize() const
{
  return _archivesize;
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

unsigned PatternImpl::mediaNr() const
{
  return _media_nr;
}


///////////////////////////////////////

TranslatedText PatternImpl::category()
{
  return _category;
}

bool PatternImpl::userVisible()
{
  return _visible;
}

Label PatternImpl::order()
{
  return _order;
}

Pathname PatternImpl::icon()
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
