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

  const ResolvableTraits::KindType ResTraits<Package>  ::_kind( "Package" );
  const ResolvableTraits::KindType ResTraits<Selection>::_kind( "Selection" );
  const ResolvableTraits::KindType ResTraits<Product>  ::_kind( "Product" );
  const ResolvableTraits::KindType ResTraits<Patch>    ::_kind( "Patch" );
  const ResolvableTraits::KindType ResTraits<Script>   ::_kind( "Script" );
  const ResolvableTraits::KindType ResTraits<Message>  ::_kind( "Message" );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
