/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Repo.h
 *
*/
#ifndef ZYPP_SAT_REPO_H
#define ZYPP_SAT_REPO_H

#include <iosfwd>

#include "zypp/base/SafeBool.h"

#include "zypp/sat/detail/PoolMember.h"
#include "zypp/sat/Solvable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Pathname;

  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Repo
    //
    /** */
    class Repo : protected detail::PoolMember,
                 private base::SafeBool<Repo>
    {
      public:
        typedef filter_iterator<detail::ByRepo, detail::SolvableIterator> SolvableIterator;
        typedef detail::size_type size_type;

      public:
        /** Default ctor creates \ref norepo.*/
        Repo()
        : _id( detail::noRepoId ) {}

        /** \ref PoolImpl ctor. */
        explicit Repo( detail::RepoIdType id_r )
        : _id( id_r ) {}

      public:
        /** Represents no \ref Repo. */
        static const Repo norepo;

        /** Evaluate \ref Repo in a boolean context (\c != \c norepo). */
        using base::SafeBool<Repo>::operator bool_type;

        /** Return whether this is the system repo. */
        bool isSystemRepo() const;

      public:
        /** The repos name (alias?). */
        std::string name() const;

        /** Whether \ref Repo contains solvables. */
        bool solvablesEmpty() const;

        /** Number of solvables in \ref Repo. */
        size_type solvablesSize() const;

        /** Iterator to the first \ref Solvable. */
        SolvableIterator solvablesBegin() const;

        /** Iterator behind the last \ref Solvable. */
        SolvableIterator solvablesEnd() const;

      public:
        /** Remove this \ref Repo from it's \ref Pool. */
        void eraseFromPool();

        /** Functor calling \ref eraseFromPool. */
        struct EraseFromPool;

      public:
        /** Load \ref Solvables from a solv-file.
         * \throws Exception if loading the solv-file fails.
         */
        void addSolv( const Pathname & file_r );

        /** Add a new empt \ref Solvable to this \ref Repo. */
        detail::SolvableIdType addSolvable();

        /** Add \c count_r new empty \ref Solvable to this \ref Repo. */
        detail::SolvableIdType addSolvables( unsigned count_r );

      public:
        /** Expert backdoor. */
        ::_Repo * get() const;
        /** Expert backdoor. */
        detail::RepoIdType id() const { return _id; }
      private:
        friend base::SafeBool<Repo>::operator bool_type() const;
        bool boolTest() const { return get(); }
      private:
        detail::RepoIdType _id;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Repo Stream output */
    std::ostream & operator<<( std::ostream & str, const Repo & obj );

    /** \relates Repo */
    inline bool operator==( const Repo & lhs, const Repo & rhs )
    { return lhs.get() == rhs.get(); }

    /** \relates Repo */
    inline bool operator!=( const Repo & lhs, const Repo & rhs )
    { return lhs.get() != rhs.get(); }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Repo::EraseFromPool
    //
    /** Functor removing \ref Repo from it's \ref Pool.
     * E.g. used as dispose function in. \ref AutoDispose
     * to provide a convenient and exception safe temporary
     * \ref Repo.
     * \code
     *  sat::Pool satpool;
     *  MIL << "1 " << satpool << endl;
     *  {
     *    AutoDispose<sat::Repo> tmprepo( (sat::Repo::EraseFromPool()) );
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
    struct Repo::EraseFromPool
    {
      void operator()( Repo repo_r ) const
      { repo_r.eraseFromPool(); }
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : RepoIterator
      //
      /** */
      class RepoIterator : public boost::iterator_adaptor<
          RepoIterator                   // Derived
          , ::_Repo **                   // Base
          , Repo                         // Value
          , boost::forward_traversal_tag // CategoryOrTraversal
          , Repo                         // Reference
          >
      {
        public:
          RepoIterator()
          : RepoIterator::iterator_adaptor_( 0 )
          {}

          explicit RepoIterator( ::_Repo ** p )
          : RepoIterator::iterator_adaptor_( p )
          {}

        private:
          friend class boost::iterator_core_access;

          Repo dereference() const
          { return Repo( *base() ); }
      };
      ///////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : ByRepo
      //
      /** Functor filtering \ref Solvable by \ref Repo.*/
      struct ByRepo
      {
        public:
          ByRepo( const Repo & repo_r ) : _repo( repo_r ) {}
          ByRepo( RepoIdType id_r ) : _repo( id_r ) {}
          ByRepo() {}

          bool operator()( const Solvable & slv_r ) const
          { return slv_r.repo() == _repo; }

        private:
          Repo _repo;
      };
      ///////////////////////////////////////////////////////////////////
     /////////////////////////////////////////////////////////////////
    } // namespace detail
    ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_REPO_H
