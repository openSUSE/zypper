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

  /** \defgroup ZYPP_RESOLVABLE_SMART_POINTER_TYPES
   * Resolvable smart pointer types.
   *
   * Forward declaration of all Resolvable smart pointer
   * types provided in \c ResTraits.h (recommended in header files):
   * \code
   * #include<zypp/ResTraits.h>
   *
   * Resolvable_Ptr                      // Resolvable *
   * ResTraits<Resolvable>::PtrType      // same as above
   *
   * Resolvable_constPtr                 // const Resolvable *
   * ResTraits<Resolvable>::constPtrType // same as above
   * \endcode
   *
   * Synonym, but requires \c Resolvable.h being included:
   * \code
   * #include<zypp/Resolvable.h>
   *
   * Resolvable::Ptr        // same as Resolvable_Ptr but requires Resolvable.h
   * Resolvable::constPtr   // same as Resolvable_constPtr but requires Resolvable.h
   * \endcode
   *
   * \note When adding a \c NewResolvable type here, dont forgett to
   * put <tt>IMPL_PTR_TYPE(NewResolvable);</tt> into the \c NewResolvable.cc.
   */
  //@{
  DEFINE_PTR_TYPE( Resolvable );
  DEFINE_PTR_TYPE( ResObject );

  DEFINE_PTR_TYPE( Atom );
  DEFINE_PTR_TYPE( Package );
  DEFINE_PTR_TYPE( SrcPackage );
  DEFINE_PTR_TYPE( Selection );
  DEFINE_PTR_TYPE( Pattern );
  DEFINE_PTR_TYPE( Product );
  DEFINE_PTR_TYPE( Patch );
  DEFINE_PTR_TYPE( Script );
  DEFINE_PTR_TYPE( Message );
  DEFINE_PTR_TYPE( Language );

  DEFINE_PTR_TYPE( SystemResObject );
  //@}

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
