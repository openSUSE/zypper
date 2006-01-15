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

    CapMatch NamedCap::matches( const constPtr & rhs ) const
    {
      if ( sameKindAndRefers( rhs ) )
        {
          intrusive_ptr<const Self> namedrhs( asKind<Self>(rhs) );
          return(    _name == namedrhs->_name
                  && range().overlaps( namedrhs->range() ) );
        }
      return false;
    }

    std::string NamedCap::encode() const
    { return _name; }

    const Edition::MatchRange & NamedCap::range() const
    {
      static Edition::MatchRange _range;
      return _range;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
