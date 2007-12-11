/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Capabilities.h
 *
*/
#ifndef ZYPP_SAT_CAPABILITIES_H
#define ZYPP_SAT_CAPABILITIES_H

#include <iosfwd>

#include "zypp/sat/detail/PoolMember.h"
#include "zypp/sat/Capability.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Capabilities
    //
    /** Container of \ref Capability (currently read only).
     * \todo PREREQ iteration
     */
    class Capabilities
    {
      public:
        typedef Capability value_type;
        typedef unsigned   size_type;

      public:
        /** Default ctor */
        Capabilities()
        : _begin( 0 )
        {}

        /** Ctor from Id pointer (for \ref Solvable). */
        explicit
        Capabilities( const detail::IdType * base_r )
        : _begin( base_r )
        {}

      public:
        /** Whether the container is empty. */
        bool empty() const
        { return ! ( _begin && *_begin ); }

        /** Number of capabilities inside. */
        size_type size() const;

      public:
        class const_iterator;

        /** Iterator pointing to the first \ref Capability. */
        const_iterator begin() const;

        /** Iterator pointing bhind the last \ref Capability. */
        const_iterator end() const;

      private:
        const detail::IdType * _begin;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Capabilities Stream output */
    std::ostream & operator<<( std::ostream & str, const Capabilities & obj );

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Capabilities::const_iterator
    //
    /** \ref Capabilities iterator.
     */
    class Capabilities::const_iterator : public boost::iterator_adaptor<
          const_iterator                     // Derived
          , const detail::IdType *           // Base
          , const Capability                 // Value
          , boost::forward_traversal_tag     // CategoryOrTraversal
          , const Capability &               // Reference
          >
    {
      public:
        const_iterator()
        : const_iterator::iterator_adaptor_( 0 )
        {}

        explicit const_iterator( const detail::IdType * _idx )
        : const_iterator::iterator_adaptor_( _idx )
        {}

      private:
        friend class boost::iterator_core_access;

        reference dereference() const
        { return (_val = Capability( *base() )); }

        template <class OtherDerived, class OtherIterator, class V, class C, class R, class D>
        bool equal( const boost::iterator_adaptor<OtherDerived, OtherIterator, V, C, R, D> & rhs ) const
        { // NULL pointer is eqal pointer to Id 0
          return ( base() == rhs.base()
                   || ( !base()     && !*rhs.base() )
                   || ( !rhs.base() && !*base()     ) );
        }

      private:
        mutable Capability _val;
    };
    ///////////////////////////////////////////////////////////////////

    inline Capabilities::const_iterator Capabilities::begin() const
    { return const_iterator( _begin ); }

    inline Capabilities::const_iterator Capabilities::end() const
    { return const_iterator( 0 ); }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_CAPABILITIES_H
