/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/memory/PackageImpl.cc
 *
*/

#include "zypp/repo/memory/PackageImpl.h"

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
//	METHOD NAME : PackageImpl::PackageImpl
//	METHOD TYPE : Ctor
//
PackageImpl::PackageImpl( repo::memory::RepoImpl::Ptr repo, data::Package_Ptr ptr)
    : _repository(repo),
                  
    _summary(ptr->summary),
    _description(ptr->description),
    _insnotify(ptr->insnotify),
    _delnotify(ptr->delnotify),
    _license_to_confirm(ptr->licenseToConfirm),
    _vendor(ptr->vendor),
    _size(ptr->installedSize),
    _archivesize(ptr->repositoryLocation.fileSize),
    _install_only(false),
    _buildtime(ptr->buildTime),
    _media_nr(ptr->repositoryLocation.mediaNr),

    _group(ptr->group),
    _keywords(),
    _authors(ptr->authors),
    _license(ptr->license),
    _buildtime(ptr->buildTime),
    _location(ptr->repositoryLocation.filePath),
    _diskusage(),
    _checksum(ptr->repositoryLocation.fileChecksum)
{
}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : PackageImpl::~PackageImpl
//	METHOD TYPE : Dtor
//
PackageImpl::~PackageImpl()
{}

Repository
PackageImpl::repository() const
{
  return _repository->selfRepository();
}

///////////////////////////////////////////////////
// ResObject Attributes
///////////////////////////////////////////////////

TranslatedText PackageImpl::summary()
{
  return _summary;
}

TranslatedText PackageImpl::description()
{
  return _description;
}

TranslatedText PackageImpl::insnotify()
{
  return _insnotify;
}

TranslatedText PackageImpl::delnotify()
{
  return _delnotify;
}

TranslatedText PackageImpl::licenseToConfirm()
{
  return _license_to_confirm;
}

Vendor PackageImpl::vendor()
{
  return _vendor;
}

ByteCount PackageImpl::size()
{
  return _size;
}

ByteCount PackageImpl::archivesize()
{
  return _archivesize;
}

bool PackageImpl::installOnly()
{
  return _install_only;
}

Date PackageImpl::buildtime()
{
  return _buildtime;
}

Date PackageImpl::installtime()
{
  return _installtime;
}

unsigned PackageImpl::mediaNr()
{
  return _media_nr;
}

////////////////////////////////////////////////////

Source_Ref PackageImpl::source() const
{
  return Source_Ref::noSource;
}

CheckSum PackageImpl::checksum() const
{
  return _checksum;
}

Date PackageImpl::buildtime() const
{
  return _buildtime;
}

string PackageImpl::buildhost() const
{
  return string();
}

Date PackageImpl::installtime() const
{
  return Date();
}				// it was never installed

string PackageImpl::distribution() const
{
  return string();
}

Vendor PackageImpl::vendor() const
{
  return string();
}

Label PackageImpl::license() const
{
  return _license;
}

string PackageImpl::packager() const
{
  return string();
}

PackageGroup PackageImpl::group() const
{
  return _group;
}

PackageImpl::Keywords PackageImpl::keywords() const
{
  return _keywords;
}

Changelog PackageImpl::changelog() const
{
  return Changelog();
}

Pathname PackageImpl::location() const
{
  return _location;
}

string PackageImpl::url() const
{
  return string();
}

string PackageImpl::os() const
{
  return string();
}

Text PackageImpl::prein() const
{
  return Text();
}

Text PackageImpl::postin() const
{
  return Text();
}

Text PackageImpl::preun() const
{
  return Text();
}

Text PackageImpl::postun() const
{
  return Text();
}

ByteCount PackageImpl::sourcesize() const
// FIXME
{
  return 0;
}

DiskUsage PackageImpl::diskusage() const
{
  return _diskusage;
}

list<string> PackageImpl::authors() const
{
  return list<string>();
}

list<string> PackageImpl::filenames() const
{
  return list<string>();
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

