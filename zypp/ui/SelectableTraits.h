/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ui/SelectableTraits.h
 *
*/
#ifndef ZYPP_UI_SELECTABLETRAITS_H
#define ZYPP_UI_SELECTABLETRAITS_H

#include <set>

#include "zypp/base/Iterator.h"
#include "zypp/PoolItem.h"
#include "zypp/ResObject.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SelectableTraits
    //
    /** */
    struct SelectableTraits
    {
      /** Oder on AvialableItemSet.
       * \li best Arch
       * \li best Edition
       * \li ResObject::constPtr as fallback.
      */
      struct AVOrder : public std::binary_function<PoolItem,PoolItem,bool>
      {
        // NOTE: operator() provides LESS semantics to order the set.
        // So LESS means 'prior in set'. We want 'better' archs and
        // 'better' editions at the beginning of the set. So we return
        // TRUE if (lhs > rhs)!
        //
        bool operator()( const PoolItem & lhs, const PoolItem & rhs ) const
        {
          int res = lhs->arch().compare( rhs->arch() );
          if ( res )
            return res > 0;
          res = lhs->edition().compare( rhs->edition() );
          if ( res )
            return res > 0;

          // no more criteria, still equal:
          // use the ResObject::constPtr (the poiner value)
          // (here it's arbitrary whether < or > )
          return lhs.resolvable() < rhs.resolvable();
        }
      };

      typedef std::set<PoolItem,AVOrder>       AvialableItemSet;
      typedef AvialableItemSet::iterator       availableItem_iterator;
      typedef AvialableItemSet::const_iterator availableItem_const_iterator;
      typedef AvialableItemSet::size_type      availableItem_size_type;

      /** Transform PoolItem to ResObject::constPtr. */
      struct TransformToResObjectPtr : public std::unary_function<PoolItem,ResObject::constPtr>
      {
        ResObject::constPtr operator()( const PoolItem & obj ) const
        { return obj; }
      };

      typedef transform_iterator<TransformToResObjectPtr, availableItem_const_iterator>
              available_iterator;

  };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_SELECTABLETRAITS_H
