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

    IMPL_PTR_TYPE(NamedCap)

    const CapabilityImpl::Kind & NamedCap::kind() const
    { return CapTraits<Self>::kind; }

    CapMatch NamedCap::matches( const CapabilityImpl::constPtr & rhs ) const
    {
      if ( sameKindAndRefers( rhs ) )
        {
          intrusive_ptr<const Self> namedrhs( asKind<Self>(rhs) );
          return(    _name == namedrhs->_name
                  && range().overlaps( namedrhs->range() ) );
        }
      return false;
    }

    bool NamedCap::same( const CapabilityImpl_constPtr & rhs ) const
    {
      intrusive_ptr<const Self> namedrhs( asKind<Self>(rhs) );
      if ( ! ( namedrhs && sameRefers( namedrhs ) ) )
        return false;

      if ( name() != namedrhs->name() )
        return false;

      const Edition::MatchRange & myrange( range() );
      const Edition::MatchRange & otherrange( namedrhs->range() );

      if ( myrange.op != otherrange.op )
        return false;

      if ( myrange.op == Rel::ANY ) // ANY ==> editions are irrelevant
        return true;

      return( myrange.value == otherrange.value );
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
