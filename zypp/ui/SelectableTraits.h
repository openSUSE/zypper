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
      /** Oder on AvalableItemSet.
       * \li repository priority
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
          unsigned lprio = lhs->satSolvable().repository().info().priority();
          unsigned rprio = rhs->satSolvable().repository().info().priority();
          if ( lprio != rprio )
            return( lprio < rprio ); // lower value meands higher priority :(
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

      typedef std::set<PoolItem,AVOrder>       AvailableItemSet;
      typedef AvailableItemSet::iterator       available_iterator;
      typedef AvailableItemSet::const_iterator available_const_iterator;
      typedef AvailableItemSet::size_type      available_size_type;

      typedef std::set<PoolItem,AVOrder>       InstalledItemSet;
      typedef AvailableItemSet::iterator       installed_iterator;
      typedef AvailableItemSet::const_iterator installed_const_iterator;
      typedef AvailableItemSet::size_type      installed_size_type;

  };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_SELECTABLETRAITS_H
