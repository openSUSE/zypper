/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/ResTraits.h
 *
*/
#ifndef ZYPP_RESTRAITS_H
#define ZYPP_RESTRAITS_H

#include "zypp/ResKind.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  template<typename _Res>
    class ResTraits;

  class Package;
  template<>
    class ResTraits<Package>
    {
    public:
      static const ResKind _kind;
    };

  class Selection;
  template<>
    class ResTraits<Selection>
    {
    public:
      static const ResKind _kind;
    };

  class Product;
  template<>
    class ResTraits<Product>
    {
    public:
      static const ResKind _kind;
    };

  class Patch;
  template<>
    class ResTraits<Patch>
    {
    public:
      static const ResKind _kind;
    };

  class Script;
  template<>
    class ResTraits<Script>
    {
    public:
      static const ResKind _kind;
    };

  class Message;
  template<>
    class ResTraits<Message>
    {
    public:
      static const ResKind _kind;
    };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESTRAITS_H
