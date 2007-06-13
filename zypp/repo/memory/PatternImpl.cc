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

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : PatternImpl::~PatternImpl
//	METHOD TYPE : Dtor
//
PatternImpl::~PatternImpl()
{}

TranslatedText PatternImpl::summary()
{
  return _summary;
}

TranslatedText PatternImpl::description()
{
  return _description;
}

TranslatedText PatternImpl::insnotify()
{
  return _insnotify;
}

TranslatedText PatternImpl::delnotify()
{
  return _delnotify;
}

TranslatedText PatternImpl::licenseToConfirm()
{
  return _license_to_confirm;
}

Vendor PatternImpl::vendor()
{
  return _vendor;
}

ByteCount PatternImpl::size()
{
  return _size;
}

ByteCount PatternImpl::archivesize()
{
  return _archivesize;
}

bool PatternImpl::installOnly()
{
  return _install_only;
}

Date PatternImpl::buildtime()
{
  return _buildtime;
}

Date PatternImpl::installtime()
{
  return _installtime;
}

unsigned PatternImpl::mediaNr()
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
