/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/capability/CapTraits.cc
 *
*/

#include "zypp/capability/CapTraits.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    template<>
      const CapabilityTraits::KindType CapTraits<NullCap>       ::kind( "NullCap" );
    template<>
      const CapabilityTraits::KindType CapTraits<FileCap>       ::kind( "FileCap" );
    template<>
      const CapabilityTraits::KindType CapTraits<NamedCap>      ::kind( "NamedCap" );
    template<>                                               // VersionedCap IsA NamedCap
      const CapabilityTraits::KindType CapTraits<VersionedCap>  ::kind( "NamedCap" );
    template<>
      const CapabilityTraits::KindType CapTraits<SplitCap>      ::kind( "SplitCap" );
    template<>
      const CapabilityTraits::KindType CapTraits<HalCap>        ::kind( "HalCap" );
    template<>
      const CapabilityTraits::KindType CapTraits<ModaliasCap>   ::kind( "ModaliasCap" );
    template<>
      const CapabilityTraits::KindType CapTraits<OrCap>         ::kind( "OrCap" );
    template<>
      const CapabilityTraits::KindType CapTraits<ConditionalCap>::kind( "ConditionalCap" );

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
