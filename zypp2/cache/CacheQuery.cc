/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <sqlite3.h>
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"

#include "zypp/base/Logger.h"

#include "zypp2/cache/DatabaseTypes.h"
#include "zypp2/cache/CacheInitializer.h"
#include "zypp2/cache/CacheQuery.h"

#include "zypp2/cache/sqlite_detail/CacheSqlite.h"
#include "zypp2/cache/sqlite_detail/CacheQueryImpl.h"
#include "zypp2/cache/sqlite_detail/CapabilityQueryImpl.h"

using namespace std;
using namespace zypp;
using namespace zypp::capability;
using namespace zypp::cache;
using namespace sqlite3x;

namespace zypp { namespace cache {

///////////////////////////////////////////////////////////////
// CACHE QUERY                                              //
//////////////////////////////////////////////////////////////

CacheQuery::CacheQuery( const Pathname &dbdir )
    : _pimpl( new Impl(dbdir) )
{
}

CacheQuery::CacheQuery( Impl *impl )
    : _pimpl( impl )
{
}

CapabilityQuery CacheQuery::createCapabilityQuery( const data::RecordId &resolvable_id  )
{
  return CapabilityQuery( new CapabilityQuery::Impl( _pimpl->context, resolvable_id) );
}

CacheQuery::~CacheQuery()
{
}

} } // ns zypp::cache


