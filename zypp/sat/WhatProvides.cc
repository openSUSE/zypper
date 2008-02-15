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

  WhatProvides::WhatProvides( Capability cap_r )
  : _begin( myPool().whatProvides( cap_r ) )
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
