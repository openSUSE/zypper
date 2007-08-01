/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/ResTraits.cc
 *
*/

#include "zypp/ResTraits.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  template<>
    const ResolvableTraits::KindType ResTraits<Package>   ::kind( "Package" );
  template<>
    const ResolvableTraits::KindType ResTraits<SrcPackage>::kind( "SrcPackage" );
  template<>
    const ResolvableTraits::KindType ResTraits<Selection> ::kind( "Selection" );
  template<>
    const ResolvableTraits::KindType ResTraits<Pattern>   ::kind( "Pattern" );
  template<>
    const ResolvableTraits::KindType ResTraits<Product>   ::kind( "Product" );
  template<>
    const ResolvableTraits::KindType ResTraits<Patch>     ::kind( "Patch" );
  template<>
    const ResolvableTraits::KindType ResTraits<Script>    ::kind( "Script" );
  template<>
    const ResolvableTraits::KindType ResTraits<Message>   ::kind( "Message" );
  template<>
    const ResolvableTraits::KindType ResTraits<Language>  ::kind( "Language" );
  template<>
    const ResolvableTraits::KindType ResTraits<Atom>	  ::kind( "Atom" );

  template<>
    const ResolvableTraits::KindType ResTraits<SystemResObject>::kind( "System" );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
