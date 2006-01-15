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
    { return CapTraits<Self>::kind; }

    bool NullCap::relevant() const
    { return false; }

    CapMatch NullCap::matches( const constPtr & rhs ) const
    { return CapMatch::irrelevant; }

    std::string NullCap::encode() const
    { return std::string(); }

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
