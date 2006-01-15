/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/NullCap.h
 *
*/
#ifndef ZYPP_CAPABILITY_NULLCAP_H
#define ZYPP_CAPABILITY_NULLCAP_H

#include "zypp/capability/CapabilityImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : NullCap
    //
    /** A dummy Capability.
     *
     * It's a singleton, so you can't construct one. Call \ref instance to
     * get a CapabilityImpl_Ptr to the NullCap.
    */
    class NullCap : public CapabilityImpl
    {
    public:
      /** Get a Ptr to the NULLCap. */
      static CapabilityImpl_Ptr instance();

    private:
      /** Private Ctor.
       * Call \ref instance to get a CapabilityImpl_Ptr to the NullCap.
      */
      NullCap();

    public:
      typedef NullCap Self;

      /**  */
      virtual const Kind & kind() const;

      /** Not relevant. */
      virtual bool relevant() const;

      /** Iirrelevant. */
      virtual CapMatch matches( const constPtr & rhs ) const;

      /** Empty string. */
      virtual std::string encode() const;

    private:
      /** Singleton */
      static CapabilityImpl_Ptr _instance;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_NULLCAP_H
