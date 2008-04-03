/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/LookupAttr.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/sat/detail/PoolImpl.h"

#include "zypp/sat/LookupAttr.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    LookupAttr::iterator LookupAttr::begin() const
    {
#warning pool search still disabled.
      if ( ! (_attr && _repo ) )
        return iterator();

      ::Dataiterator di;
      if ( _solv )
        ::dataiterator_init( &di, _solv.repository().id(), _solv.id(), _attr.id(), 0, SEARCH_NO_STORAGE_SOLVABLE );
      else
        ::dataiterator_init( &di, _repo.id(), 0, _attr.id(), 0, SEARCH_NO_STORAGE_SOLVABLE );
      return iterator( /*di*/ );
    }

    LookupAttr::iterator LookupAttr::end() const
    {
      return iterator();
    }

    std::ostream & operator<<( std::ostream & str, const LookupAttr & obj )
    {
      if ( ! obj.attr() )
        return str << "search nothing";

      str << "seach " << obj.attr() << " in ";
      if ( obj.solvable() )
        return str << obj.solvable();
      if ( obj.repo() )
        return str << obj.repo();
      return str << "pool";
    }

   ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LookupAttr::iterator
    //
    ///////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
