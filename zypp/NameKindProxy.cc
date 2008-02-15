/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/NameKindProxy.cc
 *
*/
#include <iostream>

#include "zypp/base/Easy.h"
#include "zypp/NameKindProxy.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace
  {
    /** \todo Move it to base/IOStream.h */
    template<class _Tp>
      struct PrintOn : public std::unary_function<_Tp, bool>
      {
        bool operator()( const _Tp & obj )
        {
          _str << _prfx << obj << std::endl;
          return true;
        }

        PrintOn( std::ostream & str, const std::string & prfx = std::string() )
        : _str( str )
        , _prfx( prfx )
        {}
        std::ostream & _str;
        std::string _prfx;
      };
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : NameKindProxy
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : NameKindProxy::NameKindProxy
  //	METHOD TYPE : Ctor
  //
  NameKindProxy::NameKindProxy( ResPool pool_r, const C_Str & name_r, Resolvable::Kind kind_r )
    : _kind( kind_r )
    , _name( name_r )
    {
      for_( it, pool_r.byIdentBegin( kind_r, _name ), pool_r.byIdentEnd( kind_r, _name ) )
      {
        if ( it->status().isInstalled() )
          _installed.insert( *it ) ;
        else
          _available.insert( *it );
      }
    }

   NameKindProxy::NameKindProxy( ResPool pool_r, IdString name_r, Resolvable::Kind kind_r )
    : _kind( kind_r )
    , _name( name_r )
    {
      for_( it, pool_r.byIdentBegin( kind_r, _name ), pool_r.byIdentEnd( kind_r, _name ) )
      {
        if ( it->status().isInstalled() )
          _installed.insert( *it ) ;
        else
          _available.insert( *it );
      }
    }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const NameKindProxy & obj )
  {
    str << "[" << obj.kind() << "] " << obj.name()
        << " {" << obj.installedSize() << "/" << obj.availableSize() << "}" << endl;
    std::for_each( obj.installedBegin(), obj.installedEnd(), PrintOn<PoolItem>(str,"  ") );
    std::for_each( obj.availableBegin(), obj.availableEnd(), PrintOn<PoolItem>(str,"  ") );
    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
