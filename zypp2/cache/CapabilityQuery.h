/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/


#ifndef ZYPP_CACHE_CAPABILITY_QUERY_H
#define ZYPP_CACHE_CAPABILITY_QUERY_H

#include <iosfwd>
#include <string>
#include <utility>

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
    struct CapabilityQuery
    {
    public:
      struct Result
      {
        Result( const data::RecordId &p_resolvable_id,
            //    const data::RecordId &p_id,
                const zypp::Dep &p_dep,
                capability::CapabilityImpl::Ptr p_cap )
        : resolvable_id(p_resolvable_id),
          //id(p_id),
          dep(p_dep),
          cap(p_cap)
        {}
        data::RecordId resolvable_id;
        //data::RecordId id;
        zypp::Dep dep;
        capability::CapabilityImpl::Ptr cap;
      };
     /**
      * reads the next item for the query
      * returns false if there are no
      * more results
      */
      bool read();
      /**
      * true if the last \ref read()
      * had a new result
      */
      bool valid() const;
      /**
      * Return a \ref CapabilityImpl::Ptr
      * from the current result.
      */
      Result value();
      
      /** Implementation. */
      class Impl;
     /**
      * Constructor
      */
      CapabilityQuery( Impl *impl );
     /**
      * Destructor
      */
      ~CapabilityQuery();
    private:
      
      /** Pointer to implementation. */
      RW_pointer<Impl> _pimpl;
    };

  } //NS cache
} //NS zypp

#endif

