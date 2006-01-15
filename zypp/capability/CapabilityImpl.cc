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
    bool CapabilityImpl::capImplOrderLess( const constPtr & rhs ) const
    {
      return encode() < rhs->encode();
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CapabilityImpl::capImplOrderLess
    //	METHOD TYPE : bool
    //
    std::ostream & CapabilityImpl::dumpOn( std::ostream & str ) const
    {
      return str << '[' << refers() << "] "
                 << '(' << kind() << ") "
                 << asString();
    }

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
