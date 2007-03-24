/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/SqlitePackageImpl.h
 *
*/

#include "SqlitePackageImpl.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/TranslatedText.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"

#include "schema.h"

using namespace std;
using namespace zypp::detail;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : SqlitePackageImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
SqlitePackageImpl::SqlitePackageImpl (Source_Ref source_r)
    : _source (source_r)
    , _install_only(false)
    , _size_installed(0)
    , _size_archive(0)
    , _data_loaded(false)
{}

/**
 * read package specific data from handle
 * (see SqliteSourceImpl, create_package_handle(), the handle is for the package_details table)
 * throw() on error
 */

void
SqlitePackageImpl::readHandle( sqlite_int64 id, sqlite3_stmt *handle )
{
  if ( _data_loaded )
    return;
  
  _zmdid = id;

  // 1-5: nvra, see SqliteSourceImpl
  _size_installed = sqlite3_column_int( handle, PACKAGES_TABLE_COLUMN_SIZE_INSTALLED );
  // 7: catalog
  // 8: installed
  // 9: local
  const char * text = ((const char *) sqlite3_column_text( handle, PACKAGES_TABLE_COLUMN_GROUP ));
  if (text != NULL)
    _group = text;
  _size_archive = sqlite3_column_int( handle, PACKAGES_TABLE_COLUMN_SIZE_ARCHIVE );
  text = (const char *) sqlite3_column_text( handle, PACKAGES_TABLE_COLUMN_SUMMARY );
  if (text != NULL)
    _summary = TranslatedText( string( text ) );
  text = (const char *) sqlite3_column_text( handle, PACKAGES_TABLE_COLUMN_DESCRIPTION );
  if (text != NULL)
    _description = TranslatedText( string( text ) );
  text = (const char *) sqlite3_column_text( handle, PACKAGES_TABLE_COLUMN_PACKAGE_FILENAME );	// package_filename
  
  if (text != NULL && *text != 0)
  {
    _location = Pathname( text );				// if set, use this (zmd owned source)
  }
  else
  {
    text = (const char *)sqlite3_column_text( handle, PACKAGES_TABLE_COLUMN_PACKAGE_URL );	// else use package_url
    if (text == NULL)
      ERR << "package_url NULL for id " << id << endl;
    else
      _location = Pathname( text );
  }
  _install_only = (sqlite3_column_int( handle, PACKAGES_TABLE_COLUMN_INSTALL_ONLY ) != 0);
  _media_nr = sqlite3_column_int( handle, PACKAGES_TABLE_COLUMN_MEDIA_NR );

  _data_loaded = true;
  
  return;
}


Source_Ref
SqlitePackageImpl::source() const
{
  return _source;
}

/** Package summary */
TranslatedText SqlitePackageImpl::summary() const
{
  return _summary;
}

/** Package description */
TranslatedText SqlitePackageImpl::description() const
{
  return _description;
}

PackageGroup SqlitePackageImpl::group() const
{
  return _group;
}

Pathname SqlitePackageImpl::location() const
{
  return _location;
}

ByteCount SqlitePackageImpl::size() const
{
  return _size_installed;
}

ZmdId SqlitePackageImpl::zmdid() const
{
  return _zmdid;
}

/** */
ByteCount SqlitePackageImpl::archivesize() const
{
  return _size_archive;
}

bool SqlitePackageImpl::installOnly() const
{
  return _install_only;
}

unsigned SqlitePackageImpl::sourceMediaNr() const
{
  return _media_nr;
}

Vendor SqlitePackageImpl::vendor() const
{
  return "suse";
}

/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
