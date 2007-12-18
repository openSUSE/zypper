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
    const ResKind ResTraits<Package>   ::kind( "Package" );
  template<>
    const ResKind ResTraits<SrcPackage>::kind( "SrcPackage" );
  template<>
    const ResKind ResTraits<Selection> ::kind( "Selection" );
  template<>
    const ResKind ResTraits<Pattern>   ::kind( "Pattern" );
  template<>
    const ResKind ResTraits<Product>   ::kind( "Product" );
  template<>
    const ResKind ResTraits<Patch>     ::kind( "Patch" );
  template<>
    const ResKind ResTraits<Script>    ::kind( "Script" );
  template<>
    const ResKind ResTraits<Message>   ::kind( "Message" );
  template<>
    const ResKind ResTraits<Language>  ::kind( "Language" );
  template<>
    const ResKind ResTraits<Atom>      ::kind( "Atom" );

  template<>
    const ResKind ResTraits<SystemResObject>::kind( "System" );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
