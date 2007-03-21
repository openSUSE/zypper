/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/FileCap.cc
 *
*/
#include "zypp/capability/FileCap.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    IMPL_PTR_TYPE(FileCap)
    
    const CapabilityImpl::Kind & FileCap::kind() const
    { return CapTraits<Self>::kind; }

    CapMatch FileCap::matches( const CapabilityImpl::constPtr & rhs ) const
    {
      return (    sameKindAndRefers( rhs )
               && _fname == asKind<Self>(rhs)->_fname );
    }

    std::string FileCap::encode() const
    { return _fname; }

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
