/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/ConditionalCap.h
 *
*/
#ifndef ZYPP_CAPABILITY_CONDITIONALCAP_H
#define ZYPP_CAPABILITY_CONDITIONALCAP_H

#include "zypp/capability/CapabilityImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ConditionalCap
    //
    /** \todo Implement it. */
    class ConditionalCap : public CapabilityImpl
    {
    public:
      /** Return whether the Capabilities match. */
      virtual CapMatch matches( const CapabilityImpl & rhs ) const
      { return false; }
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_CONDITIONALCAP_H
