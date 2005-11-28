/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/VersionedCap.cc
 *
*/
#include "zypp/capability/VersionedCap.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    const CapabilityImpl::Kind VersionedCap::_kind( "VersionedCap" );

    const CapabilityImpl::Kind & VersionedCap::kind() const
    { return _kind; }

    std::string VersionedCap::asString() const
    {
      std::string ret( _name );
      ret += " ";
      ret += _op.asString();
      ret += " ";
      return ret += _edition.asString();
    }

    bool VersionedCap::matches( Resolvable::constPtr resolvable_r,
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
