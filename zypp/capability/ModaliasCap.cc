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
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/capability/ModaliasCap.h"
#include "zypp/target/modalias/Modalias.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    IMPL_PTR_TYPE(ModaliasCap)
    
    /** If name_r contains 2 ':', the 1st part is a package name. */
    inline void modsplit( std::string & name_r, std::string & pkgname_r )
    {
      std::string::size_type pos1( name_r.find_first_of( ":" ) );
      std::string::size_type pos2( name_r.find_last_of( ":" ) );
      if ( pos1 != pos2 )
        {
          pkgname_r = name_r.substr( 0, pos1 );
          name_r.erase( 0, pos1+1 );
        }
    }

    /** Ctor */
    ModaliasCap::ModaliasCap( const Resolvable::Kind & refers_r,
                              const std::string & name_r )
    : CapabilityImpl( refers_r )
    , _name( name_r )
    { modsplit( _name, _pkgname ); }

    /** Ctor */
    ModaliasCap::ModaliasCap( const Resolvable::Kind & refers_r,
                              const std::string & name_r,
                              Rel op_r,
                              const std::string & value_r )
    : CapabilityImpl( refers_r )
    , _name( name_r )
    , _op( op_r )
    , _value( value_r )
    { modsplit( _name, _pkgname ); }

    const CapabilityImpl::Kind & ModaliasCap::kind() const
    { return CapTraits<Self>::kind; }

    CapMatch ModaliasCap::matches( const CapabilityImpl::constPtr & rhs ) const
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
      if ( !_pkgname.empty() )
        {
          ret += _pkgname;
          ret += ":";
        }
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
      return "modalias()";
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
