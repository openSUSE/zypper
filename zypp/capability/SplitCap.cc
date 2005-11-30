/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/SplitCap.cc
 *
*/
#include "zypp/capability/SplitCap.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    const CapabilityImpl::Kind SplitCap::_kind( "SplitCap" );

    const CapabilityImpl::Kind & SplitCap::kind() const
    { return _kind; }

    std::string SplitCap::asString() const
    { return _name + ":" + _path; }

    bool SplitCap::matches( Resolvable::constPtr resolvable_r,
                            solver::Context_constPtr solverContext_r ) const
    {
      return false;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
