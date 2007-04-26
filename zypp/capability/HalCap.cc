/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/HalCap.cc
 *
*/
#include "zypp/capability/HalCap.h"
#include "zypp/target/hal/Hal.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    IMPL_PTR_TYPE(HalCap)
    
    const CapabilityImpl::Kind & HalCap::kind() const
    { return CapTraits<Self>::kind; }

    CapMatch HalCap::matches( const CapabilityImpl::constPtr & rhs ) const
    {
      if ( sameKindAndRefers( rhs ) )
        {
          intrusive_ptr<const Self> halrhs( asKind<Self>(rhs) );
          if ( isEvalCmd() == halrhs->isEvalCmd() )
            return CapMatch::irrelevant;

          return( isEvalCmd() ? halrhs->evaluate() : evaluate() );
        }
      return false;
    }

    std::string HalCap::encode() const
    {
      std::string ret( "hal(" );
      ret += _name;
      ret += ")";
      if ( _op != Rel::ANY )
        {
          ret += " ";
          ret += _op.asString();
          ret += " ";
          ret += _value;
        }
      return ret;
    }

    std::string HalCap::index() const
    {
      return "hal()";
    }

    bool HalCap::isEvalCmd() const
    { return _name.empty(); }

    bool HalCap::evaluate() const
    {
      return target::hal::Hal::instance().query( _name, _op, _value );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
