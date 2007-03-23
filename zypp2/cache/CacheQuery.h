/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/


#ifndef ZYPP_CACHE_QUERY_H
#define ZYPP_CACHE_QUERY_H

#include <iosfwd>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/Pathname.h"

#include "zypp/capability/CapabilityImpl.h"
#include "zypp/capability/Capabilities.h"

#include "zypp/data/ResolvableDataConsumer.h"
#include "zypp/data/RecordId.h"

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////
 
    struct CapabilityQuery
    {
    public:
      class Impl;
      CapabilityQuery( Impl * );
      bool read();
      bool valid() const;
    private:
      /** Implementation. */
      /** Pointer to implementation. */
      RW_pointer<Impl> _pimpl;
    };
    
    /**
     * The Cache Query API provides access to the store data
    */
    class CacheQuery
    {
    public:
      CacheQuery( const Pathname &dbdir );
      ~CacheQuery();
      CapabilityQuery createCapabilityQuery();
    private:
      /** Implementation. */
      class Impl;
      /** Pointer to implementation. */
      RW_pointer<Impl> _pimpl;
    };
    
  } // ns cache
} // ns zypp
#endif