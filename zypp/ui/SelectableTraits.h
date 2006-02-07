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
      typedef std::set<PoolItem>               AvialableItemSet;
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
