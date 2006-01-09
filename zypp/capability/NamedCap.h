/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/NamedCap.h
 *
*/
#ifndef ZYPP_CAPABILITY_NAMEDCAP_H
#define ZYPP_CAPABILITY_NAMEDCAP_H

#include "zypp/capability/CapabilityImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : NamedCap
    //
    /** A \c name matching if some Resolvable provides it. */
    class NamedCap : public CapabilityImpl
    {
    public:
      typedef NamedCap Self;

      /** Ctor */
      NamedCap( const Resolvable::Kind & refers_r, const std::string & name_r )
      : CapabilityImpl( refers_r )
      , _name( name_r )
      {}
    public:
      /**  */
      virtual const Kind & kind() const;

      /** Return whether the Capabilities match. */
      virtual CapMatch matches( const constPtr & rhs ) const;

      /**  */
      virtual std::string asString() const;

    protected:
      /** Implementation dependent value. */
      virtual std::string value() const;

    private:
      /**  */
      std::string _name;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_NAMEDCAP_H
