/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/


#ifndef ZYPP_CACHE_CAPABILITY_QUERY_IMPL_H
#define ZYPP_CACHE_CAPABILITY_QUERY_IMPL_H

#include <iosfwd>
#include <string>
#include <utility>

#include "zypp2/cache/sqlite3x/sqlite3x.hpp"
#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/Pathname.h"
#include "zypp/data/RecordId.h"
#include "zypp/base/PtrTypes.h"
#include "zypp2/cache/sqlite_detail/CacheSqlite.h"
#include "zypp2/cache/QueryFactory.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////
 
    class CapabilityQuery::Impl
    {
      public:
      Impl( DatabaseContext_Ptr p_context, const data::RecordId &resolvable_id  );
      ~Impl();
      
      DatabaseContext_Ptr context;
      sqlite3x::sqlite3_reader_ptr _versioned_reader;
      sqlite3x::sqlite3_reader_ptr _named_reader;
      sqlite3x::sqlite3_reader_ptr _file_reader;
      data::RecordId _resolvable_id;
      bool _vercap_read;
      bool _namedcap_read;
      bool _filecap_read;
      
      bool _vercap_done;
      bool _namedcap_done;
      bool _filecap_done;
    };

  } //NS cache
} //NS zypp

#endif


