/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/WhatProvides.h
 *
*/
#ifndef ZYPP_SAT_WHATPROVIDES_H
#define ZYPP_SAT_WHATPROVIDES_H

#include <iosfwd>
#include <vector>

#include "zypp/base/PtrTypes.h"
#include "zypp/sat/detail/PoolMember.h"
#include "zypp/sat/Solvable.h"
#include "zypp/sat/SolvIterMixin.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    namespace detail
    {
      class WhatProvidesIterator;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : WhatProvides
    //
    /** Container of \ref Solvable providing a \ref Capability (read only).
     *
     * \code
     * Capability cap("amarok < 1.13");
     *
     * WhatProvides q( cap );
     * Solvable firstMatch;
     *
     * if ( ! q.empty() )
     * {
     *   cout << "Found " << q.size() << " matches for " << cap << ":" << endl;
     *   firstMatch = *q.begin();
     *
     *   for_( it, q.begin(), q.end() )
     *     cout << *it << endl;
     * }
     *
     * if ( firstMatch )
     * {
     *   WhatProvides req( firstMatch.requires() );
     *   if ( ! req.empty() )
     *   {
     *      cout << "Found " << req.size() << " items providing requirements of " << firstMatch << ":" << endl;
     *   }
     * }
     * \endcode
     *
     * \note Note that there are capabilities which are not provided by any \ref Solvable,
     * but are system properties. For example:
     * \code
     *   rpmlib(PayloadIsBzip2) <= 3.0.5-1
     * \endcode
     * In that case a \ref Solvable::noSolvable is returned, which has \c isSystem set \c true, although
     * there should never be a \ref Solvable::noSolvable returned with \c isSystem set \c false. If so,
     * please file a bugreport.
     * \code
     * WhatProvides q( Capability("rpmlib(PayloadIsBzip2) <= 3.0.5-1") );
     * for_( it, q.begin(), q.end() )
     * {
     *   if ( *it )
     *     cout << "Capability is provided by package " << *it << endl;
     *   else if ( it->isSystem() )
     *     cout << "Capability is a system property" << endl;
     *   else
     *     ; // never reaching this \c else
     * }
     * \endcode
     */
    class WhatProvides : public SolvIterMixin<WhatProvides,detail::WhatProvidesIterator>,
                         protected detail::PoolMember
    {
      public:
        typedef Solvable  value_type;
        typedef unsigned  size_type;

      public:
        /** Default ctor */
        WhatProvides()
        : _begin( 0 )
        {}

        /** Ctor from \ref Capability. */
        explicit
        WhatProvides( Capability cap_r );

        /** Ctor collecting all providers of capabilities in \c caps_r. */
        explicit
        WhatProvides( Capabilities caps_r );

        /** Ctor collecting all providers of capabilities in \c caps_r. */
        explicit
        WhatProvides( const CapabilitySet & caps_r );

     public:
        /** Whether the container is empty. */
        bool empty() const
        { return ! ( _begin && *_begin ); }

        /** Number of solvables inside. */
        size_type size() const;

      public:
        typedef detail::WhatProvidesIterator const_iterator;

        /** Iterator pointing to the first \ref Solvable. */
        const_iterator begin() const;

        /** Iterator pointing behind the last \ref Solvable. */
        const_iterator end() const;

      private:
        const sat::detail::IdType * _begin;
        shared_ptr<void> _private;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates WhatProvides Stream output */
    std::ostream & operator<<( std::ostream & str, const WhatProvides & obj );

    namespace detail
    {
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : WhatProvides::const_iterator
    //
    /** \ref WhatProvides iterator.
     * Iterate a NULL terminated sat::detail::IdType array.
     */
    class WhatProvidesIterator : public boost::iterator_adaptor<
          WhatProvidesIterator         // Derived
        , const detail::IdType *       // Base
        , const Solvable               // Value
        , boost::forward_traversal_tag // CategoryOrTraversal
        , const Solvable               // Reference
        >
    {
      public:
        WhatProvidesIterator()
        : iterator_adaptor_( 0 )
        {}

        explicit WhatProvidesIterator( const sat::detail::IdType * _idx )
        : iterator_adaptor_( _idx )
        {}

      private:
        friend class boost::iterator_core_access;

        reference dereference() const
        { return ( base() ) ? Solvable( *base() ) : Solvable::noSolvable; }

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
    }

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
