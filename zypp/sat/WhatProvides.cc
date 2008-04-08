/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/WhatProvides.cc
 *
*/
#include <iostream>

#include "zypp/base/LogTools.h"
#include "zypp/sat/WhatProvides.h"
#include "zypp/sat/detail/PoolImpl.h"

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

      /** WhatProvides ctor helper collecting providers from Capabilies. */
      template <class Iterator>
      shared_ptr<void> allocatedProviders( Iterator begin_r, Iterator end_r, const sat::detail::IdType *& first_r )
      {
        // use a set to unify the collected results
        std::tr1::unordered_set<sat::detail::IdType> ids;
        for_( it, begin_r, end_r )
        {
          WhatProvides providers( *it );
          for_( prv, providers.begin(), providers.end() )
          {
            ids.insert( prv->id() );
          }
        }

        if ( ids.empty() )
        {
          return shared_ptr<void>();
        }

        // use allocated private data to store the result
        std::vector<sat::detail::IdType> * pdata = new std::vector<sat::detail::IdType>( ids.size(), sat::detail::noId );
        pdata->insert( pdata->begin(), ids.begin(), ids.end() );
        first_r = &pdata->front();
        return shared_ptr<void>( pdata );
      }

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    WhatProvides::WhatProvides( Capability cap_r )
    : _begin( myPool().whatProvides( cap_r ) )
    {}

    WhatProvides::WhatProvides( Capabilities caps_r )
    : _begin( 0 )
    , _private( allocatedProviders( caps_r.begin(), caps_r.end(), _begin ) )
    {}

    WhatProvides::WhatProvides( const CapabilitySet & caps_r )
    : _begin( 0 )
    , _private( allocatedProviders( caps_r.begin(), caps_r.end(), _begin ) )
    {}

    WhatProvides::size_type WhatProvides::size() const
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
    std::ostream & operator<<( std::ostream & str, const WhatProvides & obj )
    {
      return dumpRange( str << "(" << obj.size() << ")", obj.begin(), obj.end() );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
