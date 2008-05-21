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
#include "zypp/base/SafeBool.h"
#include "zypp/Pathname.h"
#include "zypp/sat/detail/PoolMember.h"
#include "zypp/sat/Solvable.h"
#include "zypp/RepoInfo.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Repository
    //
    /** */
    class Repository : protected sat::detail::PoolMember,
                       private base::SafeBool<Repository>
    {
    public:
        typedef filter_iterator<detail::ByRepository, sat::detail::SolvableIterator> SolvableIterator;
        typedef sat::detail::size_type size_type;
        typedef sat::detail::RepoIdType IdType;

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
        using base::SafeBool<Repository>::operator bool_type;

        /** Return whether this is the system repository. */
        bool isSystemRepo() const;

    public:
         /** 
          * Short unique, convenience string to refer to a repo.
          * ie: openSUSE-10.3
          *
          * If you are looking for a label to display
          * see \ref info() which provides \ref RepoInfo::name()
          * ie: "openSUSE 10.3 Main repository"
          *
          */
        std::string alias() const;

         /** 
          * Short unique, convenience string to refer to a repo.
          * ie: openSUSE-10.3
          *
          * The sat solver uses name for what we know as alias
          * In rpm repositories, name is a label string
          * ie: "openSUSE 10.3 Main repository"
          *
          * We know follow rpm conventions and ignore satsolver
          * wording for name.
          *
          * Use \ref alias() instead
          */
        ZYPP_DEPRECATED std::string name() const
        { return alias(); }
        

        /** Whether \ref Repository contains solvables. */
        bool solvablesEmpty() const;

        /** Number of solvables in \ref Repository. */
        size_type solvablesSize() const;

        /** Iterator to the first \ref Solvable. */
        SolvableIterator solvablesBegin() const;

        /** Iterator behind the last \ref Solvable. */
        SolvableIterator solvablesEnd() const;

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

        /** Add \c count_r new empty \ref Solvable to this \ref Repository. */
        sat::Solvable::IdType addSolvables( unsigned count_r );
        /** \overload Add only one new \ref Solvable. */
        sat::Solvable::IdType addSolvable()
	    { return addSolvables( 1 ); }
        //@}

    public:
        /** Expert backdoor. */
        ::_Repo * get() const;
        /** Expert backdoor. */
        IdType id() const { return _id; }
    private:
        friend base::SafeBool<Repository>::operator bool_type() const;
        bool boolTest() const { return get(); }
    private:
        IdType _id;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Repository Stream output */
    std::ostream & operator<<( std::ostream & str, const Repository & obj );

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
			   , ::_Repo **                   // Base
			   , Repository                   // Value
			   , boost::forward_traversal_tag // CategoryOrTraversal
			   , Repository                   // Reference
			     >
	{
        public:
	    RepositoryIterator()
		: RepositoryIterator::iterator_adaptor_( 0 )
		{}

	    explicit RepositoryIterator( ::_Repo ** p )
		: RepositoryIterator::iterator_adaptor_( p )
		{}

        private:
	    friend class boost::iterator_core_access;

	    Repository dereference() const
		{ return Repository( *base() ); }
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

    /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_REPOSITORY_H
