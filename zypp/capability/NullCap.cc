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

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    NullCap::NullCap()
    : CapabilityImpl( Resolvable::Kind() ) // no Kind!
    {}

    CapabilityImpl_Ptr NullCap::instance()
    {
      static CapabilityImpl_Ptr _instance( new NullCap );
      return _instance;
    }

    const CapabilityImpl::Kind & NullCap::kind() const
    { return CapTraits<Self>::kind; }

    bool NullCap::relevant() const
    { return false; }

    CapMatch NullCap::matches( const CapabilityImpl::constPtr & rhs ) const
    { return CapMatch::irrelevant; }

    std::string NullCap::encode() const
    { return std::string(); }

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
