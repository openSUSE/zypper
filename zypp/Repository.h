/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Repository.h
 *
*/
#ifndef ZYPP_SAT_REPOSITORY_H
#define ZYPP_SAT_REPOSITORY_H

#include <iosfwd>
#include "zypp/base/Iterator.h"
#include "zypp/Pathname.h"
#include "zypp/sat/detail/PoolMember.h"
#include "zypp/sat/LookupAttr.h"     // LookupAttrTools.h included at EOF
#include "zypp/sat/Solvable.h"
#include "zypp/RepoInfo.h"
#include "zypp/Date.h"
#include "zypp/CpeId.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

    namespace detail
    {
      struct ByRepository;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Repository
    //
    /** */
    class Repository : protected sat::detail::PoolMember
    {
    public:
        typedef filter_iterator<detail::ByRepository, sat::detail::SolvableIterator> SolvableIterator;
        typedef sat::detail::size_type size_type;
        typedef sat::detail::RepoIdType IdType;

        typedef sat::ArrayAttr<std::string,std::string> Keywords;

	typedef std::string ContentRevision;
	typedef std::string ContentIdentifier;

    public:
        /** Default ctor creates \ref noRepository.*/
        Repository()
        : _id( sat::detail::noRepoId ) {}

        /** \ref PoolImpl ctor. */
        explicit Repository( IdType id_r )
        : _id( id_r ) {}

    public:
        /** Represents no \ref Repository. */
        static const Repository noRepository;

        /** Evaluate \ref Repository in a boolean context (\c != \c noRepository). */
        explicit operator bool() const
        { return get() != nullptr; }

        /** Reserved system repository alias \c @System. */
        static const std::string & systemRepoAlias();

        /** Return whether this is the system repository. */
        bool isSystemRepo() const;

    public:
         /**
          * Short unique string to identify a repo.
          * ie: openSUSE-10.3
          *
          * If you are looking for a label to display
          * see \ref name().
          * ie: "openSUSE 10.3 Main repository"
          *
          */
        std::string alias() const;

        /** Label to display for this repo. */
        std::string name() const;

	/** Alias or name, according to \ref ZConfig::repoLabelIsAlias */
	std::string label() const;

	/** User string: \ref label (alias or name) */
	std::string asUserString() const
	{ return label(); }

    public:
	/** Timestamp or arbitrary user supplied string.
	 * \c /repomd/revision/text() in \c repomd.xml.
	 */
	ContentRevision contentRevision() const;

	/** Unique string identifying a repositories content.
	 * \c /repomd/tags/repo/text() in \c repomd.xml.
	 * \code
	 * <repomd ....>
	 *  <tags>
	 *   <repo>obsrepository://build.suse.de/SUSE:Factory:Head:Internal/standard</repo>
	 * \endcode
	 * Semantically the value is just a plain string, even
	 * if OBS often uses the location of the project as
	 * unique identifyer.
	 */
	ContentIdentifier contentIdentifier() const;

	/** Whether \a id_r matches this repos content identifier. */
	bool hasContentIdentifier( const ContentIdentifier & id_r ) const;

        /**
         * Timestamp when this repository was generated
         *
         * Usually this value is calculated as the newer
         * timestamp from the timestamp of all the resources
         * that conform the repository's metadata.
         *
         * For example in a rpm-md repository, it would be
         * the resource specified in the xml file whith
         * the newest timestamp attribute (which is the
         * timestamp of the file in the server ).
         *
         * The timestamp is 0 if the repository does not
         * specify when it was generated.
         *
         */
        Date generatedTimestamp() const;

        /**
         * Suggested expiration timestamp.
         *
         * Repositories can define an amount of time
         * they expire, with the generated timestamp as
         * the base point of time.
         *
         * Note that is the responsability of the repository
         * to freshen the generated timestamp to tell the
         * client that the repo is alive and updating the
         * metadata.
         *
         * The timestamp is 0 if the repository does not specify
         * an expiration date.
         *
         */
        Date suggestedExpirationTimestamp() const;

        /**
         * repository keywords (tags)
         */
        Keywords keywords() const;

	/** Whether \a val_r is present in keywords. */
	bool hasKeyword( const std::string & val_r ) const;

        /**
         * The suggested expiration date of this repository
         * already passed
         *
         * rpm-md repositories can provide this tag using the
         * expire extension tag:
         * \see http://en.opensuse.org/Standards/Rpm_Metadata#SUSE_repository_info_.28suseinfo.xml.29.2C_extensions_to_repomd.xml
         */
        bool maybeOutdated() const;

        /** Hint whether the Repo may provide updates for a product.
	 *
         * Either the repository claims to update a product via a repository updates
         * tag in it's metadata or a known product lists the repositories ContentIdentifier
	 * as required update repo.
         */
        bool isUpdateRepo() const;

        /** Hint whether the Repo may provide updates for a product identified by it's \ref CpeId
	 *
         * Either the repository claims to update a product via a repository updates
         * tag in it's metadata or a known product lists the repositories ContentIdentifier
	 * as required update repo.
	 */
        bool providesUpdatesFor( const CpeId & cpeid_r ) const;

        /** Whether \ref Repository contains solvables. */
        bool solvablesEmpty() const;

        /** Number of solvables in \ref Repository. */
        size_type solvablesSize() const;

        /** Iterator to the first \ref Solvable. */
        SolvableIterator solvablesBegin() const;

        /** Iterator behind the last \ref Solvable. */
        SolvableIterator solvablesEnd() const;

	/** Iterate the repositories Solvables. */
	Iterable<SolvableIterator> solvables() const;

    public:

      /** Query class for Repository related products */
      class ProductInfoIterator;

      /**
       * Get an iterator to the beginning of the repository
       * compatible distros.
       * \note This is only a hint. There is no guarantee that
       * the repository is built for that product.
       * \see Repository::ProductInfoIterator
       */
      ProductInfoIterator compatibleWithProductBegin() const;

      /**
       * Get an iterator to the end of the repository
       * compatible distros.
       * \see Repository::ProductInfoIterator
       */
      ProductInfoIterator compatibleWithProductEnd() const;

      /** Iterate the repository compatible distros. */
      Iterable<ProductInfoIterator> compatibleWithProduct() const;


      /**
       * Get an iterator to the beginning of distos the repository
       * provides upadates for.
       * \note This is only a hint within the repositories metadata.
       * The same realation might be expressed by a product listing
       * this repositories ContentIdentifier as required update repo.
       * \see Repository::ProductInfoIterator
       */
      ProductInfoIterator updatesProductBegin() const;

      /**
       * Get an iterator to the end of distos the repository
       * provides upadates for.
       * \see Repository::ProductInfoIterator
       */
      ProductInfoIterator updatesProductEnd() const;

      /** Iterate distos the repository provides upadates for. */
      Iterable<ProductInfoIterator> updatesProduct() const;

    public:
        /** Return any associated \ref RepoInfo. */
        RepoInfo info() const;

        /** Set \ref RepoInfo for this repository.
         * \throws Exception if this is \ref noRepository
         * \throws Exception if the \ref RepoInfo::alias
         *         does not match the \ref Repository::name.
	 */
        void setInfo( const RepoInfo & info_r );

	/** Remove any \ref RepoInfo set for this repository. */
        void clearInfo();

    public:
        /** Remove this \ref Repository from it's \ref Pool. */
        void eraseFromPool();

        /** Functor calling \ref eraseFromPool. */
        struct EraseFromPool;

   public:
        /** Return next Repository in \ref Pool (or \ref noRepository). */
        Repository nextInPool() const;

   public:
        /** \name Repository content manipulating methods.
         * \todo maybe a separate Repository/Solvable content manip interface
         * provided by the pool.
         */
        //@{
        /** Load \ref Solvables from a solv-file.
         * In case of an exception the repository remains in the \ref Pool.
         * \throws Exception if this is \ref noRepository
         * \throws Exception if loading the solv-file fails.
         * \see \ref Pool::addRepoSolv and \ref Repository::EraseFromPool
         */
        void addSolv( const Pathname & file_r );

         /** Load \ref Solvables from a helix-file.
         * Supports loading of gzip compressed files (.gz). In case of an exception
         * the repository remains in the \ref Pool.
         * \throws Exception if this is \ref noRepository
         * \throws Exception if loading the helix-file fails.
         * \see \ref Pool::addRepoHelix and \ref Repository::EraseFromPool
         */
        void addHelix( const Pathname & file_r );

       /** Add \c count_r new empty \ref Solvable to this \ref Repository. */
        sat::Solvable::IdType addSolvables( unsigned count_r );
        /** \overload Add only one new \ref Solvable. */
        sat::Solvable::IdType addSolvable()
	    { return addSolvables( 1 ); }
        //@}

    public:
        /** Expert backdoor. */
        sat::detail::CRepo * get() const;
        /** Expert backdoor. */
        IdType id() const { return _id; }
        /** libsolv internal priorities.
         * Unlike the \ref RepoInfo priority which tries to be YUM conform
         * (H[1-99]L), this one is the solvers internal priority representation.
         * It is type \c int and as one might expect it, the higher the value
         * the higher the priority. Subpriority is currently used to express
         * media preferences (\see \ref MediaPriority).
         */
        //@{
        int satInternalPriority() const;
        int satInternalSubPriority() const;
        //@}

    private:
        IdType _id;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Repository Stream output */
    std::ostream & operator<<( std::ostream & str, const Repository & obj );

    /** \relates Repository XML output */
    std::ostream & dumpAsXmlOn( std::ostream & str, const Repository & obj );

    /** \relates Repository */
    inline bool operator==( const Repository & lhs, const Repository & rhs )
    { return lhs.get() == rhs.get(); }

    /** \relates Repository */
    inline bool operator!=( const Repository & lhs, const Repository & rhs )
    { return lhs.get() != rhs.get(); }

    /** \relates Repository */
    inline bool operator<( const Repository & lhs, const Repository & rhs )
    { return lhs.get() < rhs.get(); }

    ///////////////////////////////////////////////////////////////////
    /**
     * Query class for Repository related products
     *
     * Products are identified by CpeIds within the repositories metadata.
     * \see http://en.opensuse.org/Standards/Rpm_Metadata#SUSE_repository_info_.28suseinfo.xml.29.2C_extensions_to_repomd.xml
     *
     * The iterator does not provide a dereference
     * operator so you can do * on it, but you can
     * access the attributes of each related product
     * directly from the iterator.
     *
     * \code
     * for_( it, repo.compatibleWithProductBegin(), repo.compatibleWithProductEnd() )
     * {
     *   cout << it.label() << ": " << it.cpeid() << endl;
     * }
     * \endcode
     *
     */
    class Repository::ProductInfoIterator : public boost::iterator_adaptor<
        Repository::ProductInfoIterator    // Derived
        , sat::LookupAttr::iterator        // Base
        , int                              // Value
        , boost::forward_traversal_tag     // CategoryOrTraversal
        , int                              // Reference
    >
    {
      public:
        ProductInfoIterator()
        {}

        /** Product label */
        std::string label() const;

        /** The Common Platform Enumeration name for this product. */
        CpeId cpeId() const;

      private:
        friend class Repository;
        /** Hide ctor as just a limited set of attributes is valid. */
        explicit ProductInfoIterator( sat::SolvAttr attr_r, Repository repo_r );

      private:
        friend class boost::iterator_core_access;
        int dereference() const { return 0; }
    };
    ///////////////////////////////////////////////////////////////////

    /** Iterate the repository compatible distros. */
    inline Iterable<Repository::ProductInfoIterator> Repository::compatibleWithProduct() const
    { return makeIterable( compatibleWithProductBegin(), compatibleWithProductEnd() ); }

    /** Iterate distos the repository provides upadates for. */
    inline Iterable<Repository::ProductInfoIterator> Repository::updatesProduct() const
    { return makeIterable( updatesProductBegin(), updatesProductEnd() ); }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Repository::EraseFromPool
    //
    /** Functor removing \ref Repository from it's \ref Pool.
     *
     * E.g. used as dispose function in. \ref AutoDispose
     * to provide a convenient and exception safe temporary
     * \ref Repository.
     * \code
     *  sat::Pool satpool;
     *  MIL << "1 " << satpool << endl;
     *  {
     *    AutoDispose<Repository> tmprepo( (Repository::EraseFromPool()) );
     *    *tmprepo = satpool.reposInsert( "A" );
     *    tmprepo->addSolv( "sl10.1-beta7-packages.solv" );
     *    DBG << "2 " << satpool << endl;
     *    // Calling 'tmprepo.resetDispose();' here
     *    // would keep the Repo.
     *  }
     *  MIL << "3 " << satpool << endl;
     * \endcode
     * \code
     * 1 sat::pool(){0repos|2slov}
     * 2 sat::pool(){1repos|2612slov}
     * 3 sat::pool(){0repos|2slov}
     * \endcode
     * Leaving the block without calling <tt>tmprepo.resetDispose();</tt>
     * before, will automatically remove the \ref Repo from it's \ref Pool.
     */
    struct Repository::EraseFromPool
    {
	void operator()( Repository repository_r ) const
	    { repository_r.eraseFromPool(); }
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : RepositoryIterator
      //
      /** */
      class RepositoryIterator : public boost::iterator_adaptor<
	    RepositoryIterator                            // Derived
			   , sat::detail::CRepo **        // Base
                           , Repository                   // Value
			   , boost::forward_traversal_tag // CategoryOrTraversal
			   , Repository                   // Reference
			     >
      {
        public:
          RepositoryIterator()
          : RepositoryIterator::iterator_adaptor_( 0 )
          {}

          explicit RepositoryIterator( sat::detail::CRepo ** p )
          : RepositoryIterator::iterator_adaptor_( p )
          {}

        private:
          friend class boost::iterator_core_access;

          Repository dereference() const
          { return Repository( *base() ); }

          void increment();
      };
      ///////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : ByRepository
      //
      /** Functor filtering \ref Solvable by \ref Repository.*/
      struct ByRepository
      {
        public:
          ByRepository( const Repository & repository_r ) : _repository( repository_r ) {}
          ByRepository( sat::detail::RepoIdType id_r ) : _repository( id_r ) {}
          ByRepository() {}

          bool operator()( const sat::Solvable & slv_r ) const
          { return slv_r.repository() == _repository; }

        private:
          Repository _repository;
      };
      ///////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////
    } // namespace detail
    ///////////////////////////////////////////////////////////////////

    inline Iterable<Repository::SolvableIterator> Repository::solvables() const
    { return makeIterable( solvablesBegin(), solvablesEnd() ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

// Late include as sat::ArrayAttr requires Repository.h
#include "zypp/sat/LookupAttrTools.h"

#endif // ZYPP_SAT_REPOSITORY_H
