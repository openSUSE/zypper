/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_CACHE_QUERY_IMPL_H
#define ZYPP_CACHE_QUERY_IMPL_H

#include <iosfwd>
#include <string>
#include <utility>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/Pathname.h"
#include "zypp/data/RecordId.h"

#include "zypp2/cache/QueryFactory.h"
#include "zypp2/cache/sqlite_detail/CacheSqlite.h"
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////
  // CACHE QUERY                                              //
  //////////////////////////////////////////////////////////////

    struct QueryFactory::Impl
    {
      Impl( const zypp::Pathname &pdbdir );
      Impl(  const Pathname &pdbdir, sqlite3x::sqlite3_connection_ptr con );
      ~Impl();
      void initCommands();
      DatabaseContext_Ptr context;
    };

  } // ns cache
} // ns zypp

#endif

