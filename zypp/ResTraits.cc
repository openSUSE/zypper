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
    const ResolvableTraits::KindType ResTraits<Package>  ::_kind( "Package" );
  template<>
    const ResolvableTraits::KindType ResTraits<Selection>::_kind( "Selection" );
  template<>
    const ResolvableTraits::KindType ResTraits<Product>  ::_kind( "Product" );
  template<>
    const ResolvableTraits::KindType ResTraits<Patch>    ::_kind( "Patch" );
  template<>
    const ResolvableTraits::KindType ResTraits<Script>   ::_kind( "Script" );
  template<>
    const ResolvableTraits::KindType ResTraits<Message>  ::_kind( "Message" );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
