/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/NamedCap.cc
 *
*/
#include "zypp/capability/NamedCap.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    const CapabilityImpl::Kind & NamedCap::kind() const
    { return CapTraits<Self>::kind; }

    std::string NamedCap::asString() const
    { return _name; }

    CapMatch NamedCap::matches( const constPtr & rhs ) const
    {
      if ( sameRefers( rhs ) )
        {
          if ( sameKind( rhs ) )
            {
              return matchValue( rhs );
            }
          else if ( isKind<VersionedCap>( rhs ) )
            {
              // matchEditionRange in case VersionedCap has Rel::NONE
              return matchValue( rhs ) && matchEditionRange( rhs );
            }
        }
      return false;
    }

    std::string NamedCap::value() const
    { return _name; }

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
