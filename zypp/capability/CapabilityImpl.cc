/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/CapabilityImpl.cc
 *
*/
#include <iostream>

#include "zypp/capability/CapabilityImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////
    IMPL_PTR_TYPE(CapabilityImpl)

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CapabilityImpl::CapabilityImpl
    //	METHOD TYPE : Ctor
    //
    CapabilityImpl::CapabilityImpl( const Resolvable::Kind & refers_r )
    : _refers( refers_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CapabilityImpl::capImplOrderLess
    //	METHOD TYPE : bool
    //
    bool CapabilityImpl::capImplOrderLess( const CapabilityImpl::constPtr & rhs ) const
    {
      return asString() < rhs->asString();
    }

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const CapabilityImpl & obj )
    {
      return str << '[' << obj.refers() << "] "
      << '(' << obj.kind() << ") "
      << obj.asString();
    }

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
