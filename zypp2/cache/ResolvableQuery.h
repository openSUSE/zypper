
#ifndef ZYPP_CACHE_RESOLVABLE_QUERY_H
#define ZYPP_CACHE_RESOLVABLE_QUERY_H

#include "zypp/base/Function.h"
#include "zypp/Pathname.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/data/RecordId.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////
 
   /**
    * Allows to iterate around the capabilities in
    * the cache.
    * Use \ref CacheQuery to create this query.
    *
    * \code
    * CacheQuery query(dbdir);
    * CapabilityQuery capquery = query.createCapabilityQuery( zypp::Dep::REQUIRES, 1 );
    * cout << capquery.value() << endl;
    * while ( capquery.read() )
    * {
    *   // value is a CapablityImpl::Ptr
    *   cout << capquery.value() << endl;
    * }
    * \endcode
    */
    struct ResolvableQuery
    {
    public:
     /**
      * Callback definition
      * first parameter is the resolvable id.
      * second parameter is a \ref data::ResObjectData object with the resource
      */
      typedef function<bool( const data::RecordId &, const data::ResObjectData & )> ProcessResolvable;
      
      ResolvableQuery( const Pathname &dbdir, ProcessResolvable fnc );
      
      /**
      * Query by record id
      */
      void query( const data::RecordId & );
      
      /**
      * Query by matching text
      */
      void query( const std::string & );
    private:
      /** Implementation. */
      class Impl;
      /** Pointer to implementation. */
      RW_pointer<Impl> _pimpl;
    };

  } //NS cache
} //NS zypp

#endif
