/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_SolvStore_H
#define ZYPP_SolvStore_H

#include <iosfwd>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/Pathname.h"
#include "zypp/ZConfig.h"

#include "zypp/data/ResolvableDataConsumer.h"
#include "zypp/data/RecordId.h"

#include "zypp/NVRA.h"
#include "zypp/RepoStatus.h"
#include "zypp/ProgressData.h"
#include "zypp/cache/Attribute.h"

#include "satsolver/solvable.h"

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
     * SolvStore store("/path");
     * RecordId repository_id =
     *   store.lookupOrAppendRepository("some-alias");
     * store.consumePackage( repository_id, package_ptr );
     * store.commit();
     * \endcode
     *
     * \note Data will not be commited until you explicitely commit.
     */
    class SolvStore : public data::ResolvableDataConsumer
    {
    public:

      SolvStore();
      virtual ~SolvStore();

      /**
       * Constructor for the SolvStore
       *
       * \note a transaction will be started from the moment the
       * SolvStore is instanciated.
       *
       * The data will be saved in the directory specified in
       * \a dbdir. \a dbdir must exist.
       */
      SolvStore( const Pathname &solvdir );

      /**
       * Commit the changes.
       */
      void commit();

      /** \name Implementation of the \ref ResolvableDataConsumer interface. */
      //@{
      /**
       * Implementation of the \ref ResolvableDataConsumer interface
       *
       * Consume a package, inserting it in the cache, under
       * \param repository_id ownership.
       * \param package Package data
      */
      virtual data::RecordId consumePackage(const std::string &repo_id,
					    const data::Package_Ptr & package);

      /**
       * Implementation of the \ref ResolvableDataConsumer interface
       *
       * Consume a source package, inserting it in the cache, under
       * \param catalog_id ownership.
       * \param srcpackage Source package data
      */
      virtual data::RecordId consumeSourcePackage( const std::string &repo_id,
	                                           const data::SrcPackage_Ptr & srcpackage );

      /**
       * Implementation of the \ref ResolvableDataConsumer interface
       *
       * Consume a patch, inserting it in the cache, under
       * \param repository_id ownership.
       * \param patch Patch data
      */
      virtual data::RecordId consumePatch( const std::string &repo_id,
					   const data::Patch_Ptr & patch );

      /**
       * Implementation of the \ref ResolvableDataConsumer interface.
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
      virtual data::RecordId consumePackageAtom( const std::string &repo_id,
	                                         const data::PackageAtom_Ptr & atom );

      /**
       * Implementation of the \ref ResolvableDataConsumer interface
       *
       * Consume a message, inserting it in the cache, under
       * \param repository_id ownership.
       * \param message Message data
      */
      virtual data::RecordId consumeMessage( const std::string &repo_id,
					     const data::Message_Ptr & message);

      /**
       * Implementation of the \ref ResolvableDataConsumer interface
       *
       * Consume a script, inserting it in the cache, under
       * \param repository_id ownership.
       * \param script Script data
      */
      virtual data::RecordId consumeScript( const std::string & repo_id,
					    const data::Script_Ptr & script);

      /**
       * Implementation of the \ref ResolvableDataConsumer interface
       *
       * Consume a pattern, inserting it in the cache, under
       * \param repository_id ownership.
       * \param pattern Pattern data
      */
      virtual data::RecordId consumePattern( const std::string & repo_id,
					     const data::Pattern_Ptr & pattern );

      /**
       * Implementation of the \ref ResolvableDataConsumer interface
       *
       * Consume a product, inserting it in the cache, under
       * \param repository_id ownership.
       * \param pattern Pattern data
      */
      virtual data::RecordId consumeProduct( const std::string &repo_id,
					     const data::Product_Ptr & product );

      /**
       * Implementation of the \ref ResolvableDataConsumer interface
       *
       * Consume changelog of a resolvable, inserting it in the cache.
       * \param repository_id ownership.
       * \param resolvable resolvable for which the changelog data are to be saved
       * \param changelog  the changelog
       * \todo see implementation
       */
      virtual data::RecordId consumeChangelog( const data::RecordId & resolvable_id,
					       const Changelog & changelog );

      /**
       * Implementation of the \ref ResolvableDataConsumer interface
       *
       * Consume filelist of a resolvable, inserting it in the cache.
       * \param repository_id ownership.
       * \param resolvable resolvable for which the filelist is to be saved
       * \param filenames  list of filenames the resolvable contains
       * \todo see implementation
       */
      virtual data::RecordId consumeFilelist( const data::RecordId & resolvable_id,
					      const data::Filenames & filenames );

      /**
       * Implementation of the \ref ResolvableDataConsumer interface
       *
       * Consume disk usage of a resolvable, inserting it in the cache.
       *
       * Repeated entries are updated (replaced)
       *
       * \param repository_id ownership.
       * \param resolvable resolvable for which the filelist is to be saved
       * \param disk  Disk usage object
       * \todo see implementation
       */
       virtual void consumeDiskUsage( const data::RecordId &resolvable_id,
                                      const DiskUsage &disk );


      /**
       * Implementation of the \ref ResolvableDataConsumer interface
       *
       * Update a packages language specific data (summary, description,
       * EULA, ins/delnotify).
       * \param resolvable_id resolvable to be updated
       * \param data_r        Package data
       */
      virtual void updatePackageLang( const data::RecordId & resolvable_id,
				      const data::Packagebase_Ptr & data_r );
      //@}
      public:

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
       _Solvable* appendResolvable( const std::string &repo_id,
                                       const data::Resolvable_Ptr &res );

      /**
       * Insert patch RPM data into <tt>patch_packages</tt> table.
       *
       * \param prpm The patch RPM object to insert.
       * \return Record ID of the newly inserted record.
       */
      data::RecordId appendPatchRpm( const std::string &repo_id,
                                     const data::PatchRpm_Ptr & prpm);


      /**
       * Insert delta RPM data into <tt>delta_packages</tt> table.
       *
       * \param drpm The delta RPM object to insert.
       * \return Record ID of the newly inserted record.
       */
      data::RecordId appendDeltaRpm( const std::string &repo_id,
                                     const data::DeltaRpm_Ptr & drpm);


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

      void cleanRepository( const std::string &alias,
                            const ProgressData::ReceiverFnc & progressrcv );

      RepoStatus repositoryStatus( const std::string &alias );

      bool isCached( const std::string &alias );

    private:
      /** Implementation. */
      class Impl;
      /** Pointer to implementation. */
      RW_pointer<Impl> _pimpl;
    };
  }
}

#endif

