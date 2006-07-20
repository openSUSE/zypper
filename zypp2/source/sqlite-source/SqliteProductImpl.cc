/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/SqliteProductImpl.h
 *
*/

#include "SqliteProductImpl.h"
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
//        CLASS NAME : SqliteProductImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
SqliteProductImpl::SqliteProductImpl (Source_Ref source_r)
    : _source (source_r)
    , _zmdid(0)
{
}

/**
 * read package specific data from handle
 * throw() on error
 */

void
SqliteProductImpl::readHandle( sqlite_int64 id, sqlite3_stmt *handle )
{
    _zmdid = id;

    // 1-5: nvra, see SqliteSourceImpl
    // 6: status (don't care, its recomputed anyways)
    // 7: category
    const char * text = ((const char *) sqlite3_column_text( handle, 7 ));
    if (text != NULL)
	_category = text;

    return;
}


Source_Ref
SqliteProductImpl::source() const
{ return _source; }

ZmdId SqliteProductImpl::zmdid() const
{ return _zmdid; }

/** Product summary */
TranslatedText SqliteProductImpl::summary() const
{ return _summary; }

/** Product description */
TranslatedText SqliteProductImpl::description() const
{ return _description; }

/** Get the category of the product - addon or base*/
std::string SqliteProductImpl::category() const
{ return _category; }

/** Get the vendor of the product */
Label SqliteProductImpl::vendor() const
{ return _vendor; }

/** Get the name of the product to be presented to user */
Label SqliteProductImpl::displayName( const Locale & locale_r ) const
{ return _displayName; }

/** */
Url SqliteProductImpl::releaseNotesUrl() const
{ return _releaseNotesUrl; }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
