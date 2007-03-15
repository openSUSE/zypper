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

#include "zypp/capability/CapabilityImpl.h"
#include "zypp/capability/VersionedCap.h"

#include "zypp/data/ResolvableDataConsumer.h"
#include "zypp/data/RecordId.h"

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
      
      // data::ResolvableDataConsumer
      virtual void consumePackage( const data::Package &package);
      
      data::RecordId appendResolvable( const Resolvable::Kind &kind, const NVRA &nvra, const data::Dependencies &deps );
      
      void appendDependencies( const data::RecordId &, const data::Dependencies & );
      void appendDependencyList( const data::RecordId &, zypp::Dep, const std::list<capability::CapabilityImpl::constPtr> & );
      void appendDependency( const data::RecordId &, zypp::Dep, capability::CapabilityImpl::constPtr );
      
      void appendVersionedDependency( const data::RecordId &, zypp::Dep, capability::VersionedCap::constPtr);
      
      data::RecordId lookupOrAppendName( const std::string &name );
      protected:
      
      private:
        shared_ptr<sqlite3x::sqlite3_connection> _con;
        
        sqlite3x::sqlite3_command *_select_name_cmd;
        sqlite3x::sqlite3_command *_insert_name_cmd;
    };
  }
}

#endif

