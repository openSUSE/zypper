/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_CacheStore_H
#define ZYPP_CacheStore_H

#include <iosfwd>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/Pathname.h"
#include "zypp/data/ResolvableDataConsumer.h"
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////

    class CacheStore : public data::ResolvableDataConsumer
    {
      public:
      
      CacheStore();
      ~CacheStore();
      CacheStore( const Pathname &dbdir );
      
      void consumePackage( const data::Package &package );
      
      protected:
      long long insertResObject( const Resolvable::Kind &kind, const data::ResObject &res );
      void insertPackage( long long id, const data::Package &package );
      
      private:
        shared_ptr<sqlite3x::sqlite3_connection> _con;
        
        sqlite3x::sqlite3_command *_insert_package_cmd;
        sqlite3x::sqlite3_command *_insert_resolvable_cmd;
    };
  }
}

#endif