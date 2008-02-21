/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/.h
 *
*/
#ifndef ZYPP_SAT_WHATPROVIDES_H
#define ZYPP_SAT_WHATPROVIDES_H

#include <iosfwd>

#include "zypp/base/DefaultIntegral.h"
#include "zypp/sat/detail/PoolMember.h"
#include "zypp/sat/Solvable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : WhatProvides
    //
    /** Container of \ref Solvable providing a \ref Capability (read only).
     * \code
     * Capability cap("amarok < 1.13");
     * WhatProvides q( cap );
     * if ( ! q.empty() )
     * {
     *   cout << "Found " << q.size() << " matches for " << cap << ":" << endl;
     *   for_( it, q.begin(), q.end() )
     *     cout << *it << end;
     * }
     * \endcode
     */
    class WhatProvides : protected detail::PoolMember
    {
      public:
        typedef Solvable  value_type;
        typedef unsigned  size_type;

      public:
        /** Default ctor */
        WhatProvides()
        : _begin( 0 )
        {}

        /** Ctor from Id pointer (friend \ref Solvable). */
        explicit
        WhatProvides( Capability cap_r );

      public:
        /** Whether the container is empty. */
        bool empty() const
        { return ! ( _begin && *_begin ); }

        /** Number of solvables inside. */
        size_type size() const;

      public:
        class const_iterator;

        /** Iterator pointing to the first \ref Solvable. */
        const_iterator begin() const;

        /** Iterator pointing behind the last \ref Solvable. */
        const_iterator end() const;

      private:
        const sat::detail::IdType * _begin;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates WhatProvides Stream output */
    std::ostream & operator<<( std::ostream & str, const WhatProvides & obj );

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : WhatProvides::const_iterator
    //
    /** \ref WhatProvides iterator.
     */
    class WhatProvides::const_iterator : public boost::iterator_adaptor<
          const_iterator               // Derived
        , const detail::IdType *       // Base
        , const Solvable               // Value
        , boost::forward_traversal_tag // CategoryOrTraversal
        , const Solvable               // Reference
        >
    {
      public:
        const_iterator()
        : const_iterator::iterator_adaptor_( 0 )
        {}

        explicit const_iterator( const sat::detail::IdType * _idx )
        : const_iterator::iterator_adaptor_( _idx )
        {}

      private:
        friend class boost::iterator_core_access;

        reference dereference() const
        { return ( base() ) ? Solvable( *base() ) : Solvable::nosolvable; }

        template <class OtherDerived, class OtherIterator, class V, class C, class R, class D>
        bool equal( const boost::iterator_adaptor<OtherDerived, OtherIterator, V, C, R, D> & rhs ) const
        { // NULL pointer is eqal pointer to Id 0
          return ( base() == rhs.base() // includes both NULL...
              || ( !rhs.base() && !*base()     )
              || ( !base()     && !*rhs.base() ) );
        }

        void increment()
        { ++base_reference(); }
    };
    ///////////////////////////////////////////////////////////////////

    inline WhatProvides::const_iterator WhatProvides::begin() const
    { return const_iterator( _begin ); }

    inline WhatProvides::const_iterator WhatProvides::end() const
    { return const_iterator( 0 ); }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_WHATPROVIDES_H
