/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/WhatObsoletes.cc
 *
*/
#include <iostream>

#include "zypp/base/LogTools.h"
#include "zypp/sat/WhatObsoletes.h"
#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/PoolItem.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      /** WhatObsoletes ctor helper collecting obsoleted installed items. */
      shared_ptr<void> allocatedProviders( sat::Solvable item_r, const sat::detail::IdType *& first_r )
      {
        WhatProvides obsoleted( item_r.obsoletes() );
        if ( obsoleted.empty() )
        {
          return shared_ptr<void>();
        }

        // use allocated private data to store the result (incl. trailing NULL)
        std::vector<sat::detail::IdType> * pdata = 0;
        shared_ptr<void> ret;

        for_( it, obsoleted.begin(), obsoleted.end() )
        {
          if ( it->isSystem() )
          {
            if ( ! pdata )
            {
              ret.reset( (pdata = new std::vector<sat::detail::IdType>) );
            }
            pdata->push_back( it->id() );
          }
        }

        if ( pdata )
        {
          pdata->push_back( sat::detail::noId );
          first_r = &pdata->front();
        }
        return ret;
      }

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    WhatObsoletes::WhatObsoletes( Solvable item_r )
    : _begin( 0 )
    , _private( allocatedProviders( item_r, _begin ) )
    {}

    WhatObsoletes::WhatObsoletes( const PoolItem & item_r )
    : _begin( 0 )
    , _private( allocatedProviders( item_r.satSolvable(), _begin ) )
    {}

    WhatObsoletes::WhatObsoletes( const ResObject::constPtr item_r )
    : _begin( 0 )
    {
      if ( item_r )
        _private = allocatedProviders( item_r->satSolvable(), _begin );
    }

    WhatObsoletes::size_type WhatObsoletes::size() const
    {
      if ( ! _begin )
        return 0;

      Capabilities::size_type ret = 0;
      for ( const sat::detail::IdType * end = _begin; *end; ++end )
      {
        ++ret;
      }
      return ret;
    }

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const WhatObsoletes & obj )
    {
      return dumpRange( str << "(" << obj.size() << ")", obj.begin(), obj.end() );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
