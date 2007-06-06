/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/cache/CacheTypes.h
 *
*/
#ifndef ZYPP2_CACHE_CACHETYPES_H
#define ZYPP2_CACHE_CACHETYPES_H

#include <iosfwd>

#include <map>
#include <string>
#include "zypp/Rel.h"
#include "zypp/Arch.h"
#include "zypp/ResTraits.h"
#include "zypp/Dep.h"
#include "zypp/Resolvable.h"
#include "zypp/Pathname.h"
#include "zypp/data/RecordId.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CacheTypes
    //
    /** 
     * Cache types is a class that reads the dynamic types
     * from the cache, and keeps them on memory.
     * Then it can by asked for the type for an Id.
     *
     * \see CacheStore::lookupOrAppendType
     */
    class CacheTypes
    {
      friend std::ostream & operator<<( std::ostream & str, const CacheTypes & obj );
    public:
      /**
       * Default ctor 
       *
       * \param dbdir Path where the cache is located.
       *
       */
      CacheTypes( const Pathname &dbdir );
      
      /** Dtor */
      ~CacheTypes();
 
      /**
       * Relation for a cache record id.
       *
       * \param id The id you got in a cache query
       *
       * \throws Exception if the id is not a valid type
       */
      Rel relationFor( const data::RecordId &id );
      
      /**
       * Kind for a cache record id.
       *
       * \param id The id you got in a cache query
       *
       * \throws Exception if the id is not a valid type
       */
      Resolvable::Kind kindFor( const data::RecordId &id );
      
      /**
       * Dependency type for a cache record id.
       *
       * \param id The id you got in a cache query
       *
       * \throws Exception if the id is not a valid type
       */
      Dep deptypeFor( const data::RecordId &id );
      
      /**
       * Architecture for a cache record id.
       *
       * \param id The id you got in a cache query
       *
       * \throws Exception if the id is not a valid type
       */
      Arch archFor( const data::RecordId &id );
      
    public:

    private:
      void refreshCache();
      
      std::map<data::RecordId, Rel> _rel_cache;
      std::map<data::RecordId, Resolvable::Kind> _kind_cache;
      std::map<data::RecordId, std::string> _deptype_cache;
      std::map<data::RecordId, Arch> _arch_cache;
      Pathname _dbdir;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates CacheTypes Stream output */
    std::ostream & operator<<( std::ostream & str, const CacheTypes & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace cache
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP2_CACHE_CACHETYPES_H
