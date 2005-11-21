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
    /** */
    class NamedCap : public CapabilityImpl
    {
    public:
      /** Ctor */
      NamedCap( const Resolvable::Kind & refers_r, const std::string & name_r )
      : CapabilityImpl( refers_r )
      , _name( name_r )
      {}
    public:
      /**  */
      virtual const CapKind & kind() const
      { return _kind; }
      /**  */
      virtual std::string asString() const
      { return _name; }
      /**  */
      virtual bool matches( Resolvable::constPtr resolvable_r,
                            const SolverContext & colverContext_r ) const
      { return false; }
    private:
      /**  */
      static const CapKind _kind;
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
