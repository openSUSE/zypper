
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
    * 
    */
    struct ResolvableQuery
    {
    public:
     /**
      * Callback definition
      * first parameter is the resolvable id.
      * second parameter is a \ref data::ResObjectData object with the resource
      */
      typedef function<bool( const data::RecordId &, 
                             data::ResObject_Ptr )> ProcessResolvable;
      
      ResolvableQuery( const Pathname &dbdir );
      
      /**
      * Query by record id
      * \param record_id Resolvable id to query
      * \param fnc callback to send the data to. (Will be called once or none)
      */
      void query( const data::RecordId &record_id,
                  ProcessResolvable fnc  );
      
      /**
      * Query by matching text
      * \param text text to match
      * \param fnc callback to send the data to. (Will be called once per result)
      */
      void query( const std::string &text,
                  ProcessResolvable fnc  );
      
      /*void queryAttribute( const std::string &text,
                           const std::string &klass
                           const std::string 
                           ProcessResolvable fnc  );
      */
    private:
      /** Implementation. */
      class Impl;
      /** Pointer to implementation. */
      RW_pointer<Impl> _pimpl;
    };

  } //NS cache
} //NS zypp

#endif
