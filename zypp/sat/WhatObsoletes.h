/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/WhatObsoletes.h
 *
*/
#ifndef ZYPP_SAT_WHATOBSOLETES_H
#define ZYPP_SAT_WHATOBSOLETES_H

#include <iosfwd>
#include <vector>

#include "zypp/sat/WhatProvides.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : WhatObsoletes
    //
    /** Container of \b installed \ref Solvable which would be
     *  obsoleted by the \ref Solvable passed to the ctor.
     *
     * \todo Publish obsoleteUsesProvides config option.
     */
    class WhatObsoletes : public SolvIterMixin<WhatObsoletes,detail::WhatProvidesIterator>,
                          protected detail::PoolMember
    {
      public:
        typedef Solvable  value_type;
        typedef unsigned  size_type;

      public:
        /** Default ctor */
        WhatObsoletes()
        : _begin( 0 )
        {}

        /** Ctor from \ref Solvable. */
        explicit
        WhatObsoletes( Solvable item_r );

        /** Ctor from \ref PoolItem. */
        explicit
        WhatObsoletes( const PoolItem & item_r );

        /** Ctor from \ref ResObject::constPtr. */
        explicit
        WhatObsoletes( const ResObject_constPtr item_r );

        /** Ctor from a range of \ref Solvable, \ref PoolItem or \ref ResObject::constPtr. */
        template <class TIterator>
        WhatObsoletes( TIterator begin, TIterator end )
        : _begin( 0 )
        {
          for_( it, begin, end )
            ctorAdd( *it );
          ctorDone();
        }

     public:
        /** Whether the container is empty. */
        bool empty() const
        { return ! ( _begin && *_begin ); }

        /** Number of solvables inside. */
        size_type size() const;

      public:
        typedef detail::WhatProvidesIterator const_iterator;

        /** Iterator pointing to the first \ref Solvable. */
        const_iterator begin() const
        { return const_iterator( _begin ); }

        /** Iterator pointing behind the last \ref Solvable. */
        const_iterator end() const
        { return const_iterator(); }

      private:
        void ctorAdd( const PoolItem & item_r );
        void ctorAdd( ResObject_constPtr item_r );
        void ctorAdd( Solvable item_r );
        void ctorDone();

      private:
        const sat::detail::IdType * _begin;
        shared_ptr<void> _private;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates WhatObsoletes Stream output */
    std::ostream & operator<<( std::ostream & str, const WhatObsoletes & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_WHATOBSOLETES_H
