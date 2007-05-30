/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/SrcPackageImpl.h
 *
*/

#include "SrcPackageImpl.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/TranslatedText.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"

using namespace std;
using namespace zypp::detail;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : SrcPackageImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
SrcPackageImpl::SrcPackageImpl (Source_Ref source_r)
    : _source (source_r)
    , _install_only(false)
    , _size_installed(0)
    , _size_archive(0)
    , _data_loaded(false)
{}

Source_Ref
SrcPackageImpl::source() const
{
  return _source;
}

/** Package summary */
TranslatedText SrcPackageImpl::summary() const
{
  return _summary;
}

/** Package description */
TranslatedText SrcPackageImpl::description() const
{
  return _description;
}

PackageGroup SrcPackageImpl::group() const
{
  return _group;
}

Pathname SrcPackageImpl::location() const
{
  return _location;
}

ByteCount SrcPackageImpl::size() const
{
  return _size_installed;
}

/** */
ByteCount SrcPackageImpl::archivesize() const
{
  return _size_archive;
}

bool SrcPackageImpl::installOnly() const
{
  return _install_only;
}

unsigned SrcPackageImpl::sourceMediaNr() const
{
  return _media_nr;
}

Vendor SrcPackageImpl::vendor() const
{
  return "suse";
}

/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
