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
    //	CLASS NAME : NullCap
    //
    /** A dummy Capability.
     *
     * It's a singleton, so you can't construct one. Call \ref instance to
     * get a CapabilityImpl_Ptr to the NullCap.
     *
     * \todo implement matches().
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
      /**  */
      virtual const Kind & kind() const;

      /**  */
      virtual std::string asString() const;

      /**  */
      virtual bool matches( Resolvable::constPtr resolvable_r,
                            const SolverContext & colverContext_r ) const;

    private:
      /**  */
      static const Kind _kind;
      /**  */
      static CapabilityImpl_Ptr _instance;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_NAMEDCAP_H
