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
        WhatProvides();

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
        bool empty() const;

        /** Number of solvables inside. */
        size_type size() const;

      public:
        typedef detail::WhatProvidesIterator const_iterator;

        /** Iterator pointing to the first \ref Solvable. */
        const_iterator begin() const;

        /** Iterator pointing behind the last \ref Solvable. */
        const_iterator end() const;

      private:
        struct Impl;
        RW_pointer<Impl> _pimpl;
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
     * Iterate a NULL terminated sat::detail::IdType array. Ctor gets
     * the adress of a pointer to the array, and offset into the array.
     * This is needed in case the array gets reallocated.
     */
    class WhatProvidesIterator : public boost::iterator_adaptor<
          WhatProvidesIterator         // Derived
        , const detail::IdType *       // Base
        , const Solvable               // Value
        , boost::forward_traversal_tag // CategoryOrTraversal
        , const Solvable               // Reference
        >
    {
      friend std::ostream & operator<<( std::ostream & str, const WhatProvidesIterator & obj );
      public:
        WhatProvidesIterator()
        : iterator_adaptor_( 0 ), _baseRef( 0 ), _offset( 0 )
        {}

        /** Ctor with pointer to 1st elemment of an array.
         * Use otherwise unused base as pointer for _baseRef.
         */
        explicit WhatProvidesIterator( const detail::IdType *const base_r, unsigned offset_r = 0 )
        : iterator_adaptor_( base_r ), _baseRef( base_r ? &base_reference() : 0 ), _offset( offset_r )
        {}

        /** Ctor with pointer to pointer to 1st elemment of an array.
         * Required for arrays that might be relocated while iterating.
         */
        explicit WhatProvidesIterator( const detail::IdType *const* baseRef_r, unsigned offset_r )
        : iterator_adaptor_( 0 ), _baseRef( baseRef_r ), _offset( offset_r )
        {}

        /** Copy-ctor required to keep _baseRef adjusted. */
        WhatProvidesIterator( const WhatProvidesIterator & rhs )
        : iterator_adaptor_( rhs.base_reference() )
        , _baseRef( base_reference() ? &base_reference() : rhs._baseRef )
        , _offset( rhs._offset )
        {}

        /** Assignment operator required to keep _baseRef adjusted. */
        WhatProvidesIterator & operator=( const WhatProvidesIterator & rhs )
        {
          if ( this != &rhs ) // no self assign
          {
            base_reference() = rhs.base_reference();
            _baseRef = ( base_reference() ? &base_reference() : rhs._baseRef );
            _offset = rhs._offset;
          }
          return *this;
        }

      private:
        friend class boost::iterator_core_access;

        reference dereference() const
        { return Solvable( getId() ); }
#if 0
        template <class OtherDerived, class OtherIterator, class V, class C, class R, class D>
        bool equal( const boost::iterator_adaptor<OtherDerived, OtherIterator, V, C, R, D> & rhs ) const
#endif
        bool equal( const WhatProvidesIterator & rhs ) const
        {
          if ( ! ( getId() || rhs.getId() ) )
            return true; // both @end
          if ( _offset != rhs._offset )
            return false;
          if ( base_reference() )
            return( base_reference() == rhs.base_reference() );
          return( _baseRef == rhs._baseRef );
        }

        void increment()
        { ++_offset; }

        detail::IdType getId() const
        { return _baseRef ? (*_baseRef)[_offset] : detail::noId; }

      private:
        const detail::IdType *const* _baseRef;
        unsigned                     _offset;
    };
    ///////////////////////////////////////////////////////////////////
    }

    inline WhatProvides::const_iterator WhatProvides::end() const
    { return const_iterator(); }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_WHATPROVIDES_H
