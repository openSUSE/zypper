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

#include "zypp/base/PtrTypes.h"
#include "zypp/base/KindOf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Resolvable;
  class ResObject;

  class Atom;
  class Package;
  class SrcPackage;
  class Selection;
  class Pattern;
  class Product;
  class Patch;
  class Script;
  class Message;
  class Language;

  class SystemResObject;

  /** Base of ResTraits. Defines the Resolvable::Kind type. */
  struct ResolvableTraits
  {
    typedef KindOf<Resolvable>  KindType;
  };

  /** ResTraits. Defines common types and the Kind value. */
  template<typename _Res>
    struct ResTraits : public ResolvableTraits
    {
      typedef intrusive_ptr<_Res>       PtrType;
      typedef intrusive_ptr<const _Res> constPtrType;

      static const KindType kind;
    };

  /** ResTraits specialisation for Resolvable.
   * Resolvable is common base and has no Kind value.
  */
  template<>
    struct ResTraits<Resolvable> : public ResolvableTraits
    {
      typedef intrusive_ptr<Resolvable>       PtrType;
      typedef intrusive_ptr<const Resolvable> constPtrType;
    };

  /** ResTraits specialisation for ResObject.
   * ResObject is common base and has no Kind value.
  */
  template<>
    struct ResTraits<ResObject> : public ResolvableTraits
    {
      typedef intrusive_ptr<ResObject>       PtrType;
      typedef intrusive_ptr<const ResObject> constPtrType;
    };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESTRAITS_H
