/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/VersionedCap.h
 *
*/
#ifndef ZYPP_CAPABILITY_VERSIONEDCAP_H
#define ZYPP_CAPABILITY_VERSIONEDCAP_H

#include "zypp/capability/CapabilityImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : VersionedCap
    //
    /** */
    class VersionedCap : public CapabilityImpl
    {
    public:
      typedef VersionedCap Self;

      /** Ctor */
      VersionedCap( const Resolvable::Kind & refers_r,
                    const std::string & name_r,
                    Rel op_r,
                    const Edition & edition_r )
      : CapabilityImpl( refers_r )
      , _name( name_r )
      , _op( op_r )
      , _edition( edition_r )
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

      /** Implementation dependent value. */
      virtual Edition::Range editionRange() const;

    private:
      /**  */
      std::string _name;
      /**  */
      Rel _op;
      /**  */
      Edition _edition;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_VERSIONEDCAP_H
