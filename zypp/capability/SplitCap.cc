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

    IMPL_PTR_TYPE(SplitCap)
    
    const CapabilityImpl::Kind & SplitCap::kind() const
    { return CapTraits<Self>::kind; }

    CapMatch SplitCap::matches( const CapabilityImpl::constPtr & rhs ) const
    {
      return CapMatch::irrelevant;
    }

    std::string SplitCap::encode() const
    { return _name + ":" + _path; }

    ///////////////////////////////////////////////////////////////////
    //	CLASS NAME : CapabilityImpl
    ///////////////////////////////////////////////////////////////////
    /** Solver hack. */
    CapabilityImpl::SplitInfo CapabilityImpl::getSplitInfo( const Capability & cap )
    {
      SplitInfo ret;
      intrusive_ptr<const SplitCap> splitPtr( asKind<SplitCap>( cap._pimpl.getPtr() ) );
      if ( splitPtr )
        {
          ret.name = splitPtr->name();
          ret.path = splitPtr->path();
        }
      return ret;
    }
    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
