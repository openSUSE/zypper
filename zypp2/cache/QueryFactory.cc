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

#include "zypp2/cache/CacheInitializer.h"
#include "zypp2/cache/QueryFactory.h"

#include "zypp2/cache/sqlite_detail/CacheSqlite.h"
#include "zypp2/cache/sqlite_detail/QueryFactoryImpl.h"
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

QueryFactory::QueryFactory( const Pathname &dbdir )
    : _pimpl( new Impl(dbdir) )
{
}

QueryFactory::QueryFactory( Impl *impl )
    : _pimpl( impl )
{
}

CapabilityQuery QueryFactory::createCapabilityQuery()
{
  return CapabilityQuery( new CapabilityQuery::Impl( _pimpl->context ) );
}

QueryFactory::~QueryFactory()
{
}

} } // ns zypp::cache


