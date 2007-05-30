/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/CachedSourcePackageImpl.h
 *
*/

#include "CachedSourcePackageImpl.h"
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
//        CLASS NAME : CachedSourcePackageImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
CachedSourcePackageImpl::CachedSourcePackageImpl (Source_Ref source_r)
    : _source (source_r)
    , _install_only(false)
    , _size_installed(0)
    , _size_archive(0)
    , _data_loaded(false)
{}

Source_Ref
CachedSourcePackageImpl::source() const
{
  return _source;
}

/** Package summary */
TranslatedText CachedSourcePackageImpl::summary() const
{
  return _summary;
}

/** Package description */
TranslatedText CachedSourcePackageImpl::description() const
{
  return _description;
}

PackageGroup CachedSourcePackageImpl::group() const
{
  return _group;
}

Pathname CachedSourcePackageImpl::location() const
{
  return _location;
}

ByteCount CachedSourcePackageImpl::size() const
{
  return _size_installed;
}

/** */
ByteCount CachedSourcePackageImpl::archivesize() const
{
  return _size_archive;
}

bool CachedSourcePackageImpl::installOnly() const
{
  return _install_only;
}

unsigned CachedSourcePackageImpl::sourceMediaNr() const
{
  return _media_nr;
}

Vendor CachedSourcePackageImpl::vendor() const
{
  return "suse";
}

/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
