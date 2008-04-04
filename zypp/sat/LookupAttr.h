/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/LookupAttr.h
 *
*/
#ifndef ZYPP_SAT_LOOKUPATTR_H
#define ZYPP_SAT_LOOKUPATTR_H

extern "C"
{
struct _Dataiterator;
}
#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/sat/Pool.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LookupAttr
    //
    /** Lightweight attribute lookup.
     *
     * Search for an attribute \ref Pool, one \ref Repository or
     * one \ref Solvable. \ref LookupAttr builds the query,
     * \ref LookupAttr::iterator iterates over the result.
     *
     * Modifying the query will not affect any running
     * iterator.
    */
    class LookupAttr
    {
      public:
        /** Default ctor. */
        LookupAttr()
        {}
        /** Lookup \ref SolvAttr in \ref Pool (all repositories). */
        LookupAttr( SolvAttr attr_r )
        : _attr( attr_r )
        {}
        /** Lookup \ref SolvAttr in one\ref Repository. */
        LookupAttr( SolvAttr attr_r, Repository repo_r )
        : _attr( attr_r ), _repo( repo_r )
        {}
        /** Lookup \ref SolvAttr in one \ref Solvable. */
        LookupAttr( SolvAttr attr_r, Solvable solv_r )
        : _attr( attr_r ), _solv( solv_r )
        {}

      public:
        /** \name Search result. */
        //@{
        /** Result iterator. */
        class iterator;

        /** Iterator to the begin of query results. */
        iterator begin() const;

        /** Iterator behind the end of query results. */
        iterator end() const;

        //@}
      public:
        /** \name What to search. */
        //@{

        /** The \ref SolvAttr to search. */
        SolvAttr attr() const
        { return _attr; }

        /** Set the \ref SolvAttr to search. */
        void setAttr( SolvAttr attr_r )
        { _attr = attr_r; }

        //@}
      public:
        /** \name Where to search. */
        //@{
        /** Wheter to search in \ref Pool. */
        bool pool() const
        { return ! (_repo || _solv); }

        /** Set search in \ref Pool (all repositories). */
        void setPool()
        {
          _repo = Repository::noRepository;
          _solv = Solvable::noSolvable;
        }

        /** Wheter to search in one \ref Repository. */
        Repository repo() const
        { return _repo; }

        /** Set search in one \ref Repository. */
        void setRepo( Repository repo_r )
        {
          _repo = repo_r;
          _solv = Solvable::noSolvable;
        }

        /** Wheter to search in one \ref Solvable. */
        Solvable solvable() const
        { return _solv; }

        /** Set search in one \ref Solvable. */
        void setSolvable( Solvable solv_r )
         {
          _repo = Repository::noRepository;
          _solv = solv_r;
        }

        //@}
      private:
        SolvAttr   _attr;
        Repository _repo;
        Solvable   _solv;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates LookupAttr Stream output. */
    std::ostream & operator<<( std::ostream & str, const LookupAttr & obj );

    /** \relates LookupAttr Verbose stream output including the query result. */
    std::ostream & dumpOn( std::ostream & str, const LookupAttr & obj );

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LookupAttr::iterator
    //
    /** Result iterator.
     * \note Implementation: Keep iterator_adaptor base and _dip in sync!
     */
    class LookupAttr::iterator : public boost::iterator_adaptor<
        iterator                       // Derived
        , ::_Dataiterator *            // Base
        , detail::IdType               // Value
        , boost::forward_traversal_tag // CategoryOrTraversal
        , detail::IdType               // Reference
    >
    {
      public:
        iterator()
        : iterator_adaptor_( 0 )
        {}

      public:


      public:
        iterator( const iterator & rhs )
        : iterator_adaptor_( cloneFrom( rhs.base() ) )
        , _dip( base() )
        {}

        iterator & operator=( const iterator & rhs )
        {
          if ( &rhs != this )
          {
            _dip.reset( cloneFrom( rhs.base() ) );
            base_reference() = _dip.get();
          }
          return *this;
        }

      private:
        friend class LookupAttr;
        iterator( scoped_ptr< ::_Dataiterator> & dip_r )
        : iterator_adaptor_( dip_r.get() )
        {
          _dip.swap( dip_r ); // take ownership!
          increment();
        }

        ::_Dataiterator * cloneFrom( const ::_Dataiterator * rhs );

      private:
        friend class boost::iterator_core_access;

        template <class OtherDerived, class OtherIterator, class V, class C, class R, class D>
        bool equal( const boost::iterator_adaptor<OtherDerived, OtherIterator, V, C, R, D> & rhs ) const
        {
          return ( bool(base()) == bool(rhs.base()) )
              && ( ! base() || dip_equal( *base(), *rhs.base() ) );
        }

        bool dip_equal( const ::_Dataiterator & lhs, const ::_Dataiterator & rhs ) const;

        detail::IdType dereference() const;

        void increment();

      public:
        /** Expert backdoor. */
        const ::_Dataiterator * get() const
        { return _dip.get(); }
      private:
        scoped_ptr< ::_Dataiterator> _dip;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates LookupAttr::iterator Stream output. */
    std::ostream & operator<<( std::ostream & str, const LookupAttr::iterator & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_LOOKUPATTR_H
