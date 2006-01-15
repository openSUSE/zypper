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
      const CapTraitsBase::KindType CapTraits<NullCap>       ::kind( "NullCap" );
    template<>
      const CapTraitsBase::KindType CapTraits<FileCap>       ::kind( "FileCap" );
    template<>
      const CapTraitsBase::KindType CapTraits<NamedCap>      ::kind( "NamedCap" );
    template<>                                               // VersionedCap IsA NamedCap
      const CapTraitsBase::KindType CapTraits<VersionedCap>  ::kind( "NamedCap" );
    template<>
      const CapTraitsBase::KindType CapTraits<SplitCap>      ::kind( "SplitCap" );
    template<>
      const CapTraitsBase::KindType CapTraits<HalCap>        ::kind( "HalCap" );
    template<>
      const CapTraitsBase::KindType CapTraits<OrCap>         ::kind( "OrCap" );
    template<>
      const CapTraitsBase::KindType CapTraits<ConditionalCap>::kind( "ConditionalCap" );

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
