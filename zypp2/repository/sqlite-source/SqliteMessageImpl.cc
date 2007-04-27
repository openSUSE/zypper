/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/SqliteMessageImpl.h
 *
*/

#include "SqliteMessageImpl.h"
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
//        CLASS NAME : SqliteMessageImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
SqliteMessageImpl::SqliteMessageImpl (Source_Ref source_r, TranslatedText text, ZmdId zmdid)
    : _source( source_r )
    , _text( text )
    , _zmdid( zmdid )
{
}

Source_Ref
SqliteMessageImpl::source() const
{ return _source; }

TranslatedText SqliteMessageImpl::text() const
{ return _text; }

ByteCount SqliteMessageImpl::size() const
{ return _text.asString().size(); }


ZmdId SqliteMessageImpl::zmdid() const
{ return _zmdid; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
