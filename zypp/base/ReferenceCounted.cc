/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/ReferenceCounted.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/base/ReferenceCounted.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ReferenceCounted::ReferenceCounted()
    : _counter( 0 )
    {}

    ReferenceCounted::ReferenceCounted( const ReferenceCounted & /*rhs*/ )
    : _counter( 0 )
    {}

    ReferenceCounted::~ReferenceCounted()
    {
      if ( _counter )
        {
          INT << "~ReferenceCounted: nonzero reference count" << std::endl;
          throw std::out_of_range( "~ReferenceCounted: nonzero reference count" );
        }
    }

    void ReferenceCounted::unrefException() const
    {
      INT << "ReferenceCounted::unref: zero reference count" << std::endl;
      throw std::out_of_range( "ReferenceCounted::unref: zero reference count" );
    }

    std::ostream & ReferenceCounted::dumpOn( std::ostream & str ) const
    {
      return str << "ReferenceCounted(@" << (const void *)this
                 << "<=" << _counter << ")";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
