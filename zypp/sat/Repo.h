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

#include "zypp/Pathname.h"

#include "zypp/sat/Solvable.h"

///////////////////////////////////////////////////////////////////
extern "C"
{
struct _Repo;
}
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Repo
    //
    /** */
    class Repo : private base::SafeBool<Repo>
    {
      public:
        /** Ctor defaults to \ref norepo.*/
        Repo( ::_Repo * repo_r = NULL )
        : _repo( repo_r )
        {}

      public:
        /** Represents no \ref Repo. */
        static const Repo norepo;

        /** Evaluate \ref Repo in a boolean context (\c != \c norepo). */
        using base::SafeBool<Repo>::operator bool_type;

      public:
        /** The repos name (alias?). */
        const char * name() const;

        unsigned solvablesSize() const;
        SolvableIterator solvablesBegin() const;
        SolvableIterator solvablesEnd() const;

      public:
        /** Load Solvables from a solv-file.
         * \throws Exception if loading the solv-file fails.
         */
        void addSolv( const Pathname & file_r );

      public:
        /** Expert backdoor. */
        ::_Repo * get() const { return _repo; }
      private:
        friend base::SafeBool<Repo>::operator bool_type() const;
        bool boolTest() const { return _repo; }
      private:
        ::_Repo * _repo;
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

    //////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////
      /** Helper functor constructing \ref Repo. */
      struct mkRepo
      {
        typedef Repo result_type;
        result_type operator()( ::_Repo *const & ptr_r ) const
        { return ptr_r; }
      };
    } /////////////////////////////////////////////////////////////////
    // namespace detail
    //////////////////////////////////////////////////////////////////

    typedef transform_iterator<detail::mkRepo, ::_Repo **> RepoIterator;

   /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_REPO_H
