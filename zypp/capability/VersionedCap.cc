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

    const CapabilityImpl::Kind & VersionedCap::kind() const
    { return CapTraits<Self>::kind; }

    std::string VersionedCap::asString() const
    {
      std::string ret( _name );
      ret += " ";
      ret += _op.asString();
      ret += " ";
      return ret += _edition.asString();
    }

    CapMatch VersionedCap::matches( const constPtr & rhs ) const
    {
      if ( sameRefers( rhs )
           && ( sameKind( rhs ) || isKind<NamedCap>( rhs ) )
	   && sameIndex( rhs ))
        {
          return matchEditionRange( rhs );
        }
      return false;
    }

    std::string VersionedCap::value() const
    { return _name; }

    Edition::Range VersionedCap::editionRange() const
    { return Edition::Range( _op, _edition ); }

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
