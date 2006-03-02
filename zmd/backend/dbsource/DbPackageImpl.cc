/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/DbPackageImpl.h
 *
*/

#include "DbPackageImpl.h"
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
//        CLASS NAME : DbPackageImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
DbPackageImpl::DbPackageImpl (Source_Ref source_r)
    : _source (source_r)
    , _install_only(false)
    , _size_installed(0)
    , _size_archive(0)
{
}

/**
 * read package specific data from handle
 * throw() on error
 */

void
DbPackageImpl::readHandle( sqlite_int64 id, sqlite3_stmt *handle )
{
    _zmdid = id;

    // 1-5: nvra, see DbSourceImpl
    _size_installed = sqlite3_column_int( handle, 6 );
    // 7: catalog
    // 8: installed
    // 9: local
    const char * text = ((const char *) sqlite3_column_text( handle, 10 ));
    if (text != NULL)
	_group = text;
    _size_archive = sqlite3_column_int( handle, 11 );
    text = (const char *) sqlite3_column_text( handle, 12 );
    if (text != NULL)
	_summary = TranslatedText( string( text ) );
    text = (const char *) sqlite3_column_text( handle, 13 );
    if (text != NULL)
	_description = TranslatedText( string( text ) );
    text = (const char *) sqlite3_column_text( handle, 14 );
    if (text != NULL)
	_location = Pathname( text );
    _install_only = (sqlite3_column_int( handle, 15 ) != 0);

    return;
}


Source_Ref
DbPackageImpl::source() const
{ return _source; }

/** Package summary */
TranslatedText DbPackageImpl::summary() const
{ return _summary; }

/** Package description */
TranslatedText DbPackageImpl::description() const
{ return _description; }

PackageGroup DbPackageImpl::group() const
{ return _group; }

Pathname DbPackageImpl::location() const
{ return _location; }

ByteCount DbPackageImpl::size() const
{ return _size_installed; }

ZmdId DbPackageImpl::zmdid() const
{ return _zmdid; }

/** */
ByteCount DbPackageImpl::archivesize() const
{ return _size_archive; }

bool DbPackageImpl::installOnly() const
{ return _install_only; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
