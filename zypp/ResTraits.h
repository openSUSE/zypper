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
#include "zypp/ResKind.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace traits
  { /////////////////////////////////////////////////////////////////

    /** Those are denoted to be installed, if the
     *  solver verifies them as being satisfied. */
    inline bool isPseudoInstalled( ResKind kind_r )
    { return( kind_r == ResKind::patch ); }

    /////////////////////////////////////////////////////////////////
  } // namespace traits
  ///////////////////////////////////////////////////////////////////

   /** \defgroup ZYPP_RESOLVABLE_SMART_POINTER_TYPES
   * Resolvable smart pointer types.
   *
   * Forward declaration of all Resolvable smart pointer
   * types provided in \c ResTraits.h (recommended in header files):
   * \code
   * #include "zypp/ResTraits.h"
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
   * #include "zypp/Resolvable.h"
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

  DEFINE_PTR_TYPE( Package );
  DEFINE_PTR_TYPE( SrcPackage );
  DEFINE_PTR_TYPE( Pattern );
  DEFINE_PTR_TYPE( Product );
  DEFINE_PTR_TYPE( Patch );
  //@}

  /** Frequently associated. */
  class PoolItem;

  /** ResTraits. Defines common types and the ResKind value. */
  template<typename _Res>
    struct ResTraits
    {
      typedef ResKind                   KindType;
      typedef intrusive_ptr<_Res>       PtrType;
      typedef intrusive_ptr<const _Res> constPtrType;

      static const ResKind              kind;

      /** Those are denoted to be installed, if the
       *  solver verifies them as being satisfied. */
      static bool isPseudoInstalled()   { return traits::isPseudoInstalled( kind ); }
    };

  /** ResTraits specialisation for Resolvable.
   * Resolvable is common base and has no Kind value.
   */
  template<>
    struct ResTraits<Resolvable>
    {
      typedef ResKind                         KindType;
      typedef intrusive_ptr<Resolvable>       PtrType;
      typedef intrusive_ptr<const Resolvable> constPtrType;
    };

  /** ResTraits specialisation for ResObject.
   * ResObject is common base and has no Kind value.
   */
  template<>
    struct ResTraits<ResObject>
    {
      typedef ResKind                        KindType;
      typedef intrusive_ptr<ResObject>       PtrType;
      typedef intrusive_ptr<const ResObject> constPtrType;
    };

  /** Convenient access to well known ResKinds.
   * \code
   * ResKind packagekind = ResKind::package;
   * ResKind packagekind = resKind<Package>();
   * \endcode
  */
  template<typename _Res>
    inline ResKind resKind() { return ResTraits<_Res>::kind; }

  /** Convenient test for ResKinds.
   * \code
   * ResKind value;
   * if ( ResKind::package == value )
   * if ( resKind<Package>() == value )
   * if ( isKind<Package>( value ) )
   * \endcode
   */
  template<typename _Res>
    inline bool isKind( const ResKind & val_r )
    { return( resKind<_Res>() == val_r ); }
  /** \overload */
  template<typename _Res>
    inline bool isKind( const std::string & val_r )
    { return( resKind<_Res>() == val_r ); }
  /** \overload */
  template<typename _Res>
    inline bool isKind( const char * val_r )
    { return( resKind<_Res>() == val_r ); }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESTRAITS_H
