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
#include <vector>

#include "zypp/base/Iterator.h"
#include "zypp/PoolItem.h"
#include "zypp/pool/ByIdent.h"

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
      /** Oder on AvailableItemSet.
       * \li repository priority
       * \li best Arch (arch/noarch changes are ok)
       * \li best Edition
       * \li newer buildtime
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
          int lprio = lhs->satSolvable().repository().satInternalPriority();
          int rprio = rhs->satSolvable().repository().satInternalPriority();
          if ( lprio != rprio )
            return( lprio > rprio );

          // arch/noarch changes are ok.
          if ( lhs->arch() != Arch_noarch && rhs->arch() != Arch_noarch )
          {
            int res = lhs->arch().compare( rhs->arch() );
            if ( res )
              return res > 0;
          }

          int res = lhs->edition().compare( rhs->edition() );
          if ( res )
            return res > 0;

	  lprio = lhs->buildtime();
	  rprio = rhs->buildtime();
	  if ( lprio != rprio )
            return( lprio > rprio );

          lprio = lhs->satSolvable().repository().satInternalSubPriority();
          rprio = rhs->satSolvable().repository().satInternalSubPriority();
          if ( lprio != rprio )
            return( lprio > rprio );

          // no more criteria, still equal: sort by id
          return lhs.satSolvable().id() < rhs.satSolvable().id();
        }
      };

      /** Oder on InstalledItemSet.
       * \li best Arch
       * \li best Edition
       * \li newer install time
       * \li ResObject::constPtr as fallback.
      */
      struct IOrder : public std::binary_function<PoolItem,PoolItem,bool>
      {
        // NOTE: operator() provides LESS semantics to order the set.
        // So LESS means 'prior in set'. We want 'newer' install time
        // at the beginning of the set.
        //
        bool operator()( const PoolItem & lhs, const PoolItem & rhs ) const
        {
          int res = lhs->arch().compare( rhs->arch() );
          if ( res )
            return res > 0;
          res = lhs->edition().compare( rhs->edition() );
          if ( res )
            return res > 0;
          Date ldate = lhs->installtime();
          Date rdate = rhs->installtime();
          if ( ldate != rdate )
            return( ldate > rdate );

          // no more criteria, still equal: sort by id
          return lhs.satSolvable().id() < rhs.satSolvable().id();
        }
      };

      typedef std::set<PoolItem,AVOrder>       AvailableItemSet;
      typedef AvailableItemSet::iterator       available_iterator;
      typedef AvailableItemSet::const_iterator available_const_iterator;
      typedef AvailableItemSet::size_type      available_size_type;

      typedef std::set<PoolItem,IOrder>        InstalledItemSet;
      typedef AvailableItemSet::iterator       installed_iterator;
      typedef AvailableItemSet::const_iterator installed_const_iterator;
      typedef AvailableItemSet::size_type      installed_size_type;

      typedef std::vector<PoolItem>             PickList;
      typedef PickList::const_iterator          picklist_iterator;
      typedef PickList::size_type               picklist_size_type;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_SELECTABLETRAITS_H
