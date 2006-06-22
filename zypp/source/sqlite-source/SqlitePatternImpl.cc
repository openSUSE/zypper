/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/SqlitePatternImpl.h
 *
*/

#include "SqlitePatternImpl.h"
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
//        CLASS NAME : SqlitePatternImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
SqlitePatternImpl::SqlitePatternImpl (Source_Ref source_r)
    : _source (source_r)
    , _zmdid(0)
    , _default(false)
    , _visible(false)
{
}

/**
 * read package specific data from handle
 * throw() on error
 */

void
SqlitePatternImpl::readHandle( sqlite_int64 id, sqlite3_stmt *handle )
{
    _zmdid = id;

    // 1-5: nvra, see SqliteSourceImpl

    // 6: status (don't care, its recomputed anyways)

    return;
}


Source_Ref
SqlitePatternImpl::source() const
{ return _source; }

/** Pattern summary */
TranslatedText SqlitePatternImpl::summary() const
{ return _summary; }

/** Pattern description */
TranslatedText SqlitePatternImpl::description() const
{ return _description; }

bool SqlitePatternImpl::isDefault() const
{ return _default; }

bool SqlitePatternImpl::userVisible() const
{ return _visible; }

TranslatedText SqlitePatternImpl::category() const
{ return _category; }

Pathname SqlitePatternImpl::icon() const
{ return _icon; }

Pathname SqlitePatternImpl::script() const
{ return _script; }
      
Label SqlitePatternImpl::order() const
{ return _order; }

ZmdId SqlitePatternImpl::zmdid() const
{ return _zmdid; }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
