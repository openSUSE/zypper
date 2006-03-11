/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/ModaliasCap.cc
 *
*/
#include "zypp/capability/ModaliasCap.h"
#include "zypp/target/modalias/Modalias.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    const CapabilityImpl::Kind & ModaliasCap::kind() const
    { return CapTraits<Self>::kind; }

    CapMatch ModaliasCap::matches( const constPtr & rhs ) const
    {
      if ( sameKindAndRefers( rhs ) )
        {
          intrusive_ptr<const Self> modaliasrhs( asKind<Self>(rhs) );
          if ( isEvalCmd() == modaliasrhs->isEvalCmd() )
            return CapMatch::irrelevant;

          return( isEvalCmd() ? modaliasrhs->evaluate() : evaluate() );
        }
      return false;
    }

    std::string ModaliasCap::encode() const
    {
      std::string ret( "modalias(" );
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

    std::string ModaliasCap::index() const
    {
      std::string ret( "modalias(" );
      ret += _name;
      return ret += ")";
    }

    bool ModaliasCap::isEvalCmd() const
    { return _name.empty(); }

    bool ModaliasCap::evaluate() const
    {
      return target::modalias::Modalias::instance().query( _name, _op, _value );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
