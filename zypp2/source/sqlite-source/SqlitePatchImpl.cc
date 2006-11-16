/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/SqlitePatchImpl.cc
 *
*/


#include "zypp/source/SourceImpl.h"
#include "zypp/TranslatedText.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"

#include "SqlitePatchImpl.h"
#include "schema.h"

using namespace std;
using namespace zypp::detail;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : SqlitePatchImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
SqlitePatchImpl::SqlitePatchImpl (Source_Ref source_r)
    : _source (source_r)
    , _reboot_needed( false )
    , _affects_pkg_manager( false )
    , _interactive( false )
{
}

/**
 * read patch specific data from handle
 * (see SqliteSourceImpl, create_patch_handle(), the handle is for the patch_details table)
 * throw() on error


	//      6               7
	"       installed_size, catalog,"
	//      8          9      10        11
	"       installed, local, patch_id, status,"
	//      12             13        14
	"       creation_time, category, reboot,"
	//      15       16
	"       restart, interactive"
 */

void
SqlitePatchImpl::readHandle( sqlite_int64 id, sqlite3_stmt *handle )
{
    _zmdid = id;

    // 1-5: nvra, see SqliteSourceImpl
    _size_installed = sqlite3_column_int( handle, PATCHES_TABLE_COLUMN_SIZE_INSTALLED );
    // 7: catalog
    // 8: installed
    // 9: local
    const char * text = ((const char *) sqlite3_column_text( handle, PATCHES_TABLE_COLUMN_PATCH_ID ));
    if (text != NULL)
	_id = text;
    // 11: status (will re recomputed anyways)
    _timestamp = sqlite3_column_int64( handle, PATCHES_TABLE_COLUMN_TIMESTAMP );
    text = (const char *) sqlite3_column_text( handle, PATCHES_TABLE_COLUMN_CATEGORY );
    if (text != NULL)
	_category = text;

    _reboot_needed = (sqlite3_column_int( handle, PATCHES_TABLE_COLUMN_REBOOT_NEEDED ) != 0);
    _affects_pkg_manager = (sqlite3_column_int( handle, PATCHES_TABLE_COLUMN_AFFECTS_PACKAGE_MANAGER ) != 0);
    _interactive = (sqlite3_column_int( handle, PATCHES_TABLE_COLUMN_INTERACTIVE ) != 0);

    return;
}


Source_Ref
SqlitePatchImpl::source() const
{ return _source; }

ZmdId SqlitePatchImpl::zmdid() const
{ return _zmdid; }

ByteCount SqlitePatchImpl::size() const
{ return _size_installed; }

/** Patch ID */
std::string SqlitePatchImpl::id() const
{ return _id; }
/** Patch time stamp */
Date SqlitePatchImpl::timestamp() const
{ return _timestamp; }
/** Patch category (recommended, security,...) */
std::string SqlitePatchImpl::category() const
{ return _category; }
/** Does the system need to reboot to finish the update process? */
bool SqlitePatchImpl::reboot_needed() const
{ return _reboot_needed; }
/** Does the patch affect the package manager itself? */
bool SqlitePatchImpl::affects_pkg_manager() const
{ return _affects_pkg_manager; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
