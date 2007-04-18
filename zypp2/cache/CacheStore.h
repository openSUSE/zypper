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
#include "zypp/NVRA.h"
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
     * The cache store caches resolvable data into some backend.
    */
    class CacheStore
    {
    public:
      
      CacheStore();
      ~CacheStore();
      
      /**
       * Constructor for the CacheStore
       *
       * \note a transaction will be started from the moment the
       * CacheStore is instanciated.
       * 
       * The data will be saved in the directory specified in
       * \a dbdir
       */
      CacheStore( const Pathname &dbdir );
      
      /**
       * Implements the ResolvableConsumer consumePackage interface
       * Consumer a package and inserts it into the database.
       * Don't use this method yet
      */
      virtual void consumePackage( const data::Package &package);
      
      /**
       * Appends a resolvable to the store.
       *
       * You have to specify with \a kind of resolvable are you inserting
       * and its \c NVRA (name version release and architecture ).
       * Optionaly you can pass a list of \c CapabilityImpl::Ptr
       * as dependencies for the resolvable.
       * 
       * You have to specify the RecordId for the catalog owning
       * this resolvable. Yuu can obtain it with
       * \ref lookupOrAppendCatalog
       * 
       * You can create those \a deps using \ref capability::parse
       * functions, or the build methods to create specific types
       * of capabilities:
       * \ref capability::buildVersioned for \c VersionedCap
       * \ref capability::buildNamed for \c NamedCap
       * etc.
       *
       * Once the resolvable is inserted, you will get back the id
       * if it in the store. Which you can use for later adding
       * other properties.
       *
       */
      data::RecordId appendResolvable( const data::RecordId &catalog_id,
                                       const Resolvable::Kind &kind,
                                       const NVRA &nvra,
                                       const data::Dependencies &deps );
      
      /**
       * Adds dependencies to the store
       *
       * A map of dependency lists has to be specified. The map contains
       * list of capablities for each dependency type \ref zypp::Dep
       *
       * \a resolvable_id is the resolvable Id in the CacheStore
       * that will own those capabilities.
       *
       * FIXME should it \throw if the resolvable does not exist?
       */
      void appendDependencies( const data::RecordId &resolvable_id,
                               const data::Dependencies &dependencies );
      
      /**
       * Adds dependencies to the store
       *
       * A lists of dependencies \a dlist to be specified. Among
       * which type of dependencies \ref zypp::Dep it is as
       * the \a deptype argument.
       * 
       * \a resolvable_id is the resolvable Id in the CacheStore
       * that will own those capabilities.
       *
       * FIXME should it \throw if the resolvable does not exist?
       */
      void appendDependencyList( const data::RecordId &resolvable_id, 
                                 zypp::Dep deptype,
                                 const data::DependencyList &dlist );
      
      /**
       * Adds a dependency to the store.
       *
       * A \ref CapabilityImpl::Ptr argument \a cap has to be specified. 
       * Among which type of dependency \ref zypp::Dep it is as
       * the \a deptype argument.
       * 
       * \a resolvable_id is the resolvable Id in the CacheStore
       * that will own the capability
       *
       * FIXME should it \throw if the resolvable does not exist?
       */
      void appendDependency( const data::RecordId &resolvable_id,
                             zypp::Dep deptype,
                             capability::CapabilityImpl::Ptr cap );
      
      /**
       * Adds a Named dependency to the store.
       *
       * A \ref NamedCap::Ptr \a dlist to be specified. Among
       * which type of dependency \ref zypp::Dep it is as
       * the \a deptype argument.
       * 
       * \a resolvable_id is the resolvable Id in the CacheStore
       * that will own the capability
       *
       * You can create the named capability using either
       * \ref capability::parse or \ref capability::buildNamed
       *
       * FIXME should it \throw if the resolvable does not exist?
       */
      void appendNamedDependency( const data::RecordId &, zypp::Dep,
                                  capability::NamedCap::Ptr);
      
      /**
       * Adds a file dependency to the store.
       *
       * A \ref FileCap::Ptr \a dlist to be specified. Among
       * which type of dependency \ref zypp::Dep it is as
       * the \a deptype argument.
       * 
       * \a resolvable_id is the resolvable Id in the CacheStore
       * that will own the capability
       *
       * You can create the file capability using either
       * \ref capability::parse or \ref capability::buildFile
       *
       * FIXME should it \throw if the resolvable does not exist?
       */
      void appendFileDependency( const data::RecordId &, zypp::Dep, 
                                 capability::FileCap::Ptr);
      
      /**
       * Returns the record id of a catalog (Source) \a path
       *
       * \note If the catalog entry does not exist, it will
       * be created and the new inserted entry's id will
       * be returned.
       */
      data::RecordId lookupOrAppendCatalog( const Url &url, 
                                            const Pathname &path );
      
       /**
       * Update a known catalog checksum and timestamp
       *
       * \note If you don't provide timestamp it defaults
       * to now.
       *
       * It is responsability of the caller to operate with
       * a valid record id. You can get one
       * Using \ref lookupOrAppendCatalog
       */
      void updateCatalog( const data::RecordId &id, 
                                    const std::string &checksum, 
                                    const Date &timestamp = Date::now() );
      
      /**
       * Returns the record id of a file entry \a path
       *
       * \note If the file entry does not exist, it will
       * be created and the new inserted entry's id will
       * be returned.
       */
      data::RecordId lookupOrAppendFile( const Pathname &path );
      
      /**
       * Returns the record id of a name entry \a name
       *
       * \note If the name entry does not exist, it will
       * be created and the new inserted entry's id will
       * be returned.
       */
      data::RecordId lookupOrAppendName( const std::string &name );
      
      /**
       * Returns the record id of a directory name  entry \a name
       *
       * \note If the directory name entry does not exist, it will
       * be created and the new inserted entry's id will
       * be returned.
       */
      data::RecordId lookupOrAppendDirName( const std::string &name );
      
      /**
       * Returns the record id of a file name entry \a name
       *
       * \note If the file name entry does not exist, it will
       * be created and the new inserted entry's id will
       * be returned.
       */
      data::RecordId lookupOrAppendFileName( const std::string &name );
      
    protected:
      /**
       * Internally used function that appends a entry in
       * the capabilities table for a specific capability
       * entry.
       */
      data::RecordId appendDependencyEntry( const data::RecordId &, 
                                            zypp::Dep, const Resolvable::Kind & );
      
      data::RecordId lookupOrAppendNamedDependencyEntry( const data::RecordId name_id,
                                                    const Edition &edition,
                                                    const zypp::Rel &rel );
    private:
      /** Implementation. */
      class Impl;
      /** Pointer to implementation. */
      RW_pointer<Impl> _pimpl;
    };
  }
}

#endif

