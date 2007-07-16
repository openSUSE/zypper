/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/memory/SrcPackageImpl.cc
 *
*/
#include "zypp/repo/memory/SrcPackageImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace repo
{ /////////////////////////////////////////////////////////////////
namespace memory
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : SrcPackageImpl::SrcPackageImpl
//	METHOD TYPE : Ctor
//
SrcPackageImpl::SrcPackageImpl( memory::RepoImpl::Ptr repo, data::SrcPackage_Ptr ptr)
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
               
    _location(ptr->repositoryLocation)
{}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : SrcPackageImpl::~SrcPackageImpl
//	METHOD TYPE : Dtor
//
SrcPackageImpl::~SrcPackageImpl()
{}

///////////////////////////////////////////////////
// ResObject Attributes
///////////////////////////////////////////////////

TranslatedText SrcPackageImpl::summary() const
{
  return _summary;
}

TranslatedText SrcPackageImpl::description() const
{
  return _description;
}

TranslatedText SrcPackageImpl::insnotify() const
{
  return _insnotify;
}

TranslatedText SrcPackageImpl::delnotify() const
{
  return _delnotify;
}

TranslatedText SrcPackageImpl::licenseToConfirm() const
{
  return _license_to_confirm;
}

Vendor SrcPackageImpl::vendor() const
{
  return _vendor;
}

ByteCount SrcPackageImpl::size() const
{
  return _size;
}

bool SrcPackageImpl::installOnly() const
{
  return _install_only;
}

Date SrcPackageImpl::buildtime() const
{
  return _buildtime;
}

Date SrcPackageImpl::installtime() const
{
  return _installtime;
}

    
DiskUsage SrcPackageImpl::diskusage() const
{
  return _diskusage;
}

OnMediaLocation SrcPackageImpl::location() const
{
  return _location;
}

/////////////////////////////////////////////////////////////////
} // namespace memory
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
