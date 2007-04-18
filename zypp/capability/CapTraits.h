/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/capability/CapTraits.h
 *
*/
#ifndef ZYPP_CAPABILITY_CAPTRAITS_H
#define ZYPP_CAPABILITY_CAPTRAITS_H

#include "zypp/base/PtrTypes.h"
#include "zypp/base/KindOf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Capability;

  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    /** Base of CapTraits. Defines the Capability::Kind type. */
    struct CapabilityTraits
    {
      typedef KindOf<Capability>  KindType;
    };

    class NullCap;
    class FileCap;
    class NamedCap;
    class VersionedCap;
    class SplitCap;
    class HalCap;
    class ModaliasCap;
    class FilesystemCap;
    class OrCap;
    class ConditionalCap;

    /** CapTraits. Defines common types and the Kind value. */
    template<typename _Cap>
      struct CapTraits : public CapabilityTraits
      {
        static const KindType kind;
      };

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_CAPTRAITS_H
