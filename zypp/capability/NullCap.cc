/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/NullCap.cc
 *
*/
#include "zypp/capability/NullCap.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    const CapabilityImpl::Kind NullCap::_kind( "NullCap" );

    CapabilityImpl_Ptr NullCap::_instance;

    ///////////////////////////////////////////////////////////////////

    NullCap::NullCap()
    : CapabilityImpl( Resolvable::Kind() ) // no Kind!
    {}

    CapabilityImpl_Ptr NullCap::instance()
    {
      if ( ! _instance )
        _instance = new NullCap;
      return _instance;
    }

    const CapabilityImpl::Kind & NullCap::kind() const
    { return _kind; }

    std::string NullCap::asString() const
    { return std::string(); }

    bool NullCap::matches( Resolvable::constPtr resolvable_r,
                           const SolverContext & colverContext_r ) const
    {
      return false;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
