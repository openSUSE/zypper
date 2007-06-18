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
#include "zypp/ZConfig.h"
#include "zypp/capability/CapabilityImpl.h"
#include "zypp/capability/Capabilities.h"

#include "zypp/data/ResolvableDataConsumer.h"
#include "zypp/data/RecordId.h"

#include "zypp/base/PtrTypes.h"
#include "zypp/RepoStatus.h"
#include "zypp/ProgressData.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////

    /**
     * The cache store caches resolvable data.
     *
     * \code
     * CacheStore store("/path");
     * RecordId repository_id =
     *   store.lookupOrAppendRepository("some-alias");
     * store.consumePackage( repository_id, package_ptr );
     * store.commit();
     * \endcode
     *
     * \note Data will not be commited until you explicitely commit.
     */
    class CacheStore : public data::ResolvableDataConsumer
    {
    public:

      CacheStore();
      virtual ~CacheStore();

      /**
       * Constructor for the CacheStore
       *
       * \note a transaction will be started from the moment the
       * CacheStore is instanciated.
       *
       * The data will be saved in the directory specified in
       * \a dbdir. \a dbdir must exist.
       */
      CacheStore( const Pathname &dbdir );

      /**
       * Commit the changes.
       */
      void commit();

      /**
       * Implementation of the \ref ResolvableConsumer interface
       *
       * Consume a package, inserting it in the cache, under
       * \param repository_id ownership.
       * \param package Package data
      */
      virtual data::RecordId consumePackage(const data::RecordId &repository_id,
					    const data::Package_Ptr & package);

      /**
       * Implementation of the \ref ResolvableConsumer interface
       *
       * Consume a source package, inserting it in the cache, under
       * \param catalog_id ownership.
       * \param srcpackage Source package data
      */
      virtual data::RecordId consumeSourcePackage( const data::RecordId &catalog_id,
	                                           const data::SrcPackage_Ptr & srcpackage );

      /**
       * Implementation of the \ref ResolvableConsumer interface
       *
       * Consume a patch, inserting it in the cache, under
       * \param repository_id ownership.
       * \param patch Patch data
      */
      virtual data::RecordId consumePatch( const data::RecordId &repository_id,
					   const data::Patch_Ptr & patch );

      /**
       * Implementation of the \ref ResolvableConsumer interface.
       *
       * Consume a package atom, inserting it in the cache, under
       * \a repository_id ownership.
       *
       * \param repository_id record id of repository to which to append the resolvable.
       * \param atom package atom data
       *
       * \note this is somewhat specific to current YUM patch metadata design
       *       and may change (to consumeAtom(data::RecordId,data::Atom)).
       */
      virtual data::RecordId consumePackageAtom( const data::RecordId &repository_id,
	                                         const data::PackageAtom_Ptr & atom );

      /**
       * Implementation of the \ref ResolvableConsumer interface
       *
       * Consume a message, inserting it in the cache, under
       * \param repository_id ownership.
       * \param message Message data
      */
      virtual data::RecordId consumeMessage( const data::RecordId & repository_id,
					     const data::Message_Ptr & message);

      /**
       * Implementation of the \ref ResolvableConsumer interface
       *
       * Consume a script, inserting it in the cache, under
       * \param repository_id ownership.
       * \param script Script data
      */
      virtual data::RecordId consumeScript( const data::RecordId & repository_id,
					    const data::Script_Ptr & script);

      /**
       * Implementation of the \ref ResolvableConsumer interface
       *
       * Consume a pattern, inserting it in the cache, under
       * \param repository_id ownership.
       * \param pattern Pattern data
      */
      virtual data::RecordId consumePattern( const data::RecordId & repository_id,
					     const data::Pattern_Ptr & pattern );

      /**
       * Implementation of the \ref ResolvableConsumer interface
       *
       * Consume a product, inserting it in the cache, under
       * \param repository_id ownership.
       * \param pattern Pattern data
      */
      virtual data::RecordId consumeProduct( const data::RecordId &repository_id,
					     const data::Product_Ptr & product );

      /**
       * Implementation of the \ref ResolvableConsumer interface
       *
       * Consume changelog of a resolvable, inserting it in the cache.
       * \param repository_id ownership.
       * \param resolvable resolvable for which the changelog data are to be saved
       * \param changelog  the changelog
       * \todo see implementation
       */
      virtual data::RecordId consumeChangelog( const data::RecordId & repository_id,
					       const data::Resolvable_Ptr & resolvable,
					       const Changelog & changelog );

      /**
       * Implementation of the \ref ResolvableConsumer interface
       *
       * Consume filelist of a resolvable, inserting it in the cache.
       * \param repository_id ownership.
       * \param resolvable resolvable for which the filelist is to be saved
       * \param filenames  list of filenames the resolvable contains
       * \todo see implementation
       */
      virtual data::RecordId consumeFilelist( const data::RecordId &repository_id,
					      const data::Resolvable_Ptr & resolvable,
					      const data::Filenames & filenames );

      /**
       * Appends a resolvable to the store.
       *
       * You have to specify with \a kind of resolvable are you inserting
       * and its \c NVRA (name version release and architecture ).
       * Optionaly you can pass a list of \c CapabilityImpl::Ptr
       * as dependencies for the resolvable.
       *
       * You have to specify the RecordId for the repository owning
       * this resolvable. Yuu can obtain it with
       * \ref lookupOrAppendRepository
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
      data::RecordId appendResolvable( const data::RecordId &repository_id,
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
       * Adds a Modalias dependency to the store.
       *
       * A \ref ModaliasCap::Ptr \a cap to be specified. Among
       * which type of dependency \ref zypp::Dep it is as
       * the \a deptype argument.
       *
       * \a resolvable_id is the resolvable Id in the CacheStore
       * that will own the capability
       *
       * You can create the modalias capability using either
       * \ref capability::parse or \ref capability::buildModalias
       *
       * FIXME should it \throw if the resolvable does not exist?
       */
      void appendModaliasDependency( const data::RecordId &resolvable_id,
                                     zypp::Dep deptype,
                                     capability::ModaliasCap::Ptr cap);

      /**
       * Adds a Hal dependency to the store.
       *
       * A \ref HalCap::Ptr \a cap to be specified. Among
       * which type of dependency \ref zypp::Dep it is as
       * the \a deptype argument.
       *
       * \a resolvable_id is the resolvable Id in the CacheStore
       * that will own the capability
       *
       * You can create the modalias capability using either
       * \ref capability::parse or \ref capability::buildHal
       *
       * FIXME should it \throw if the resolvable does not exist?
       */
      void appendHalDependency( const data::RecordId &resolvable_id,
                                      zypp::Dep deptype,
                                      capability::HalCap::Ptr cap );

      /**
       * Adds a unknown dependency to the store.
       *
       * A \ref Capability::Ptr \a cap to be specified. Among
       * which type of dependency \ref zypp::Dep it is as
       * the \a deptype argument.
       *
       * \a resolvable_id is the resolvable Id in the CacheStore
       * that will own the capability
       *
       * You can create the capability using either
       * \ref capability::parse
       *
       * FIXME should it \throw if the resolvable does not exist?
       */
      void appendUnknownDependency( const data::RecordId &resolvable_id,
                                    zypp::Dep deptype,
                                    capability::CapabilityImpl::Ptr cap );

      /**
       * Insert patch RPM data into <tt>patch_packages</tt> table.
       *
       * \param prpm The patch RPM object to insert.
       * \return Record ID of the newly inserted record.
       */
      data::RecordId appendPatchRpm( const data::RecordId &repo_id,
                                     const data::PatchRpm_Ptr & prpm);


      /**
       * Insert delta RPM data into <tt>delta_packages</tt> table.
       *
       * \param drpm The delta RPM object to insert.
       * \return Record ID of the newly inserted record.
       */
      data::RecordId appendDeltaRpm( const data::RecordId &repo_id,
                                     const data::DeltaRpm_Ptr & drpm);


      /**
       * Returns the record id of a type
       *
       * Types are mostly used internally. To give concepts
       * a record id to associate with.
       * Examples could be arch::i386, lang::en_US
       * Packages::summary, rel:>, kind::Package
       *
       * \note If the type entry does not exist, it will
       * be created and the new inserted entry's id will
       * be returned.
       */
      data::RecordId lookupOrAppendType( const std::string &klass,
                                         const std::string &name );

      /**
       * Returns the record id of a repository (Source)
       *
       * \param alias Unique alias for this repo
       *
       * \note If the repository entry does not exist, it will
       * be created and the new inserted entry's id will
       * be returned.
       */
      data::RecordId lookupOrAppendRepository( const std::string &alias );

      /**
       * Set the resolvable shared data flag pointing to
       * another resolvable.
       *
       * This is a hint for cache readers. If any attribute
       * of a resolvable is empty, is because it is shared
       * with another resolvable.
       *
       * \param resolvable_id Id of the resolvable. Must exists
       * \param shared_id The resolvable providing the data
       * This one is a weak reference, the reader should just
       * try to look the data there as a hint.
       * use \ref data::noRecordId to reset the value.
       *
       */
      void setSharedData( const data::RecordId &resolvable_id,
                          const data::RecordId &shared_id );

      /**
       * Append a numeric attribute to a resolvable
       * \param resolvable_id Resovable Id, owner of the attribute
       * \param klass Type class i.e "Package" "lang" "kind"
       * \param name Type name i.e : "size" "media_number"
       * \param value numeric value
       */
      void appendNumericAttribute( const data::RecordId &resolvable_id,
                                   const std::string &klass,
                                   const std::string &name,
                                   int value );

      /**
       * Append a translated string value to a resolvable
       * \param resolvable_id Resovable Id, owner of the attribute
       * \param klass Type class i.e "Package" "lang" "kind"
       * \param name Type name i.e : "summary" "none" "Script"
       * \param text Translated text
       */
      void appendTranslatedStringAttribute( const data::RecordId &resolvable_id,
                                            const std::string &klass,
                                            const std::string &name,
                                            const TranslatedText &text );

      /**
       * Append a string value to a resolvable
       * \param resolvable_id Resovable Id, owner of the attribute
       * \param locale locale of the text language
       * \param klass Type class i.e "Package" "lang" "kind"
       * \param name Type name i.e : "summary" "none" "Script"
       * \param text text
       */
      void appendStringAttributeTranslation( const data::RecordId &resolvable_id,
                                             const Locale &locale,
                                             const std::string &klass,
                                             const std::string &name,
                                             const std::string &text );

      /**
       * Append a string value to a resolvable
       * \param resolvable_id Resovable Id, owner of the attribute
       * \param klass Type class i.e "Package" "lang" "kind"
       * \param name Type name i.e : "summary" "none" "Script"
       * \param value string value
       */
      void appendStringAttribute( const data::RecordId &resolvable_id,
                                  const std::string &klass,
                                  const std::string &name,
                                  const std::string &value );

      /**
       * Append a string value to a resolvable
       * \param resolvable_id Resovable Id, owner of the attribute
       * \param type_id Type id, \see lookupOrAppendType
       * \param value string value
       */
      void appendStringAttribute( const data::RecordId &resolvable_id,
                                  const data::RecordId &type_id,
                                  const std::string &value );


      /**
       * Append strings from _Iterator to a resolvable.
       *
       * Uses \ref zypp::str::split(_Iterator,_Iterator, std::string) with
       * \ref ZConfig::cacheDBSplitJoinSeparator() as the second argument
       * (a separator string) of split().
       *
       * Any container of any class providing asString() can be used.
       *
       * \param resolvable_id Resovable Id, owner of the attribute
       * \param klass Type class i.e "Package" "lang" "kind"
       * \param name Type name i.e : "summary" "none" "Script"
       * \param begin begin Iterator to the container
       * \param end end Iterator to the container
       */
      template <class _Iterator>
      void appendStringContainerAttribute( const data::RecordId &resolvable_id,
                                           const std::string &klass,
                                           const std::string &name,
                                           _Iterator begin,
                                           _Iterator end )
      {
        std::string value = str::join(begin, end, ZConfig().cacheDBSplitJoinSeparator());
        appendStringAttribute( resolvable_id, klass, name, value );
      }

      /**
       * Append strings from a _Container to a resolvable.
       *
       * Convenience method taking the container instead of it's
       * begin/end iterators as argument.
       *
       * \see appendStringContainerAttribute
      */
      template <class _Container>
      void appendStringContainerAttribute( const data::RecordId &resolvable_id,
                                           const std::string &klass,
                                           const std::string &name,
                                           const _Container & container )
      {	appendStringContainerAttribute( resolvable_id, klass, name, container.begin(), container.end() ); }

       /**
       * Update a known repository checksum and timestamp
       *
       * \note If you don't provide timestamp it defaults
       * to now.
       *
       * It is responsability of the caller to operate with
       * a valid record id. You can get one
       * Using \ref lookupOrAppendRepository
       *
       * If the repository does not exists, nothing will happen
       */
      void updateRepositoryStatus( const data::RecordId &id,
                                   const RepoStatus &status );

      /**
       * \short Clean repository from cache
       *
       * \param id repository identifier in cache
       *
       * You can check existence using \ref isCached
       *
       * \throws CacheRecordNotFoundException if the repository
       * id does not refer to a valid repository.
       */
      void cleanRepository( const data::RecordId &id,
                            const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );

      /**
       * \short Clean repository from cache
       *
       * \param alias Repository unique alias
       *
       * You can check existence using \ref isCached
       *
       * \throws CacheRecordNotFoundException if the repository
       * alias does not refer to a valid repository.
       */
      void cleanRepository( const std::string &alias,
                            const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );

      /**
       * get the status of a cached repository
       *
       * It is responsability of the caller to operate with
       * a valid record id. You can get one
       * Using \ref lookupOrAppendRepository
       *
       * You can check existence using \ref isCached
       *
       * \throws CacheRecordNotFoundException if the repository
       * id does not refer to a valid repository.
       */
      RepoStatus repositoryStatus( const data::RecordId &id );

      /**
       * get the status of a cached repository
       *
       * It is responsability of the caller to operate with
       * a valid alias. You can insert one
       * Using \ref lookupOrAppendRepository
       *
       * You can check existence using \ref isCached
       *
       * \throws CacheRecordNotFoundException if the repository
       * alias is unknown
       */
      RepoStatus repositoryStatus( const std::string &alias );

      /**
       * \short Does a repository exists in cache?
       *
       * True if the repository is cached
       */
      bool isCached( const std::string &alias );

      /**
       * \short looks the id for a repository in cache
       *
       * \param alias Repository unique alias
       *
       * \throws CacheRecordNotFoundException if the repository
       * alias is unknown
       */
      data::RecordId lookupRepository( const std::string &alias );


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
//       data::RecordId appendDependencyEntry( const data::RecordId &,
//                                             zypp::Dep, const Resolvable::Kind & );

      void appendStringAttribute( const data::RecordId &resolvable_id,
                                  const data::RecordId &lang_id,
                                  const data::RecordId &type_id,
                                  const std::string &value );

      /**
       * Append a numeric attribute to a resolvable
       * \param resolvable_id Resovable Id, owner of the attribute
       * \param type_id attribute id
       * \param value numeric value
       */
      void appendNumericAttribute( const data::RecordId &resolvable_id,
                                   const data::RecordId &type_id,
                                   int value );

      /**
       * Append a bool attribute to a resolvable. Will be stored as
       * numeric 1 or 0.
       *
       * \param resolvable_id Resovable Id, owner of the attribute
       * \param type_id attribute id
       * \param value bool value
       */
      void appendBooleanAttribute( const data::RecordId & resolvable_id,
                                   const std::string & klass,
                                   const std::string & name,
                                   bool value);

      /** \name Detail Attributes Inserters
       * These functions are used by ResolvableConsumer interface functions
       * to avoid some duplication across types.
       */
      //@{
      void appendResObjectAttributes( const data::RecordId &rid,
                                      const data::ResObject_Ptr & res );

      void appendPackageBaseAttributes(const data::RecordId & pkgid,
                                       const data::Packagebase_Ptr & package);
      //@}


    private:
      /** Implementation. */
      class Impl;
      /** Pointer to implementation. */
      RW_pointer<Impl> _pimpl;
    };
  }
}

#endif

