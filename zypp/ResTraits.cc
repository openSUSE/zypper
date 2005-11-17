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
#include "zypp/ResKind.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  const ResKind ResTraits<Package>  ::_kind( "Package" );
  const ResKind ResTraits<Selection>::_kind( "Selection" );
  const ResKind ResTraits<Product>  ::_kind( "Product" );
  const ResKind ResTraits<Patch>    ::_kind( "Patch" );
  const ResKind ResTraits<Script>   ::_kind( "Script" );
  const ResKind ResTraits<Message>  ::_kind( "Message" );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
