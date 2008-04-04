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
#include <cstring>
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
//       if ( ! (_attr ) )
//         return iterator();

      scoped_ptr< ::_Dataiterator> dip( new ::Dataiterator );
      ::memset( dip.get(), 0, sizeof(::_Dataiterator) );
      if ( _solv )
        ::dataiterator_init( dip.get(), _solv.repository().id(), _solv.id(), _attr.id(), 0, SEARCH_NO_STORAGE_SOLVABLE );
      else if ( _repo )
        ::dataiterator_init( dip.get(), _repo.id(), 0, _attr.id(), 0, SEARCH_NO_STORAGE_SOLVABLE );
      else
#warning pool search still disabled.
        return iterator(); // pool search not yet s

      return iterator( dip ); // iterator takes over ownership!
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

    std::ostream & dumpOn( std::ostream & str, const LookupAttr & obj )
    {
      str << obj << endl;
      for_( it, obj.begin(), obj.end() )
      {
        str << "  " << it << endl;
      }
      return str << "<EndOfSerach>";
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LookupAttr::iterator
    //
    ///////////////////////////////////////////////////////////////////

    ::_Dataiterator * LookupAttr::iterator::cloneFrom( const ::_Dataiterator * rhs )
    {
      if ( ! rhs )
        return 0;
      ::_Dataiterator * ret( new ::_Dataiterator );
      *ret = *rhs;
      return ret;
    }

    bool LookupAttr::iterator::dip_equal( const ::_Dataiterator & lhs, const ::_Dataiterator & rhs ) const
    {
      return ::memcmp( &lhs, &rhs, sizeof(::_Dataiterator) ) == 0;
    }

    detail::IdType LookupAttr::iterator::dereference() const
    {
      return _dip ? ::repodata_globalize_id( _dip->data, _dip->kv.id )
                  : detail::noId;
    }

    void LookupAttr::iterator::increment()
    {
      if ( _dip && ! ::dataiterator_step( _dip.get() ) )
      {
        _dip.reset();
        base_reference() = 0;
      }
    }

    std::ostream & operator<<( std::ostream & str, const LookupAttr::iterator & obj )
    {
      const ::_Dataiterator * dip = obj.get();
      if ( ! dip )
        return str << "EndOfQuery" << endl;

      str << Solvable( dip->solvid )
          << '<' << SolvAttr( dip->keyname )
          << "> = " << IdString( dip->key->type )
          << "(" <<  dip->kv.id << ")";
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
