/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResObject.h
 *
*/
#ifndef ZYPP_RESOBJECT_H
#define ZYPP_RESOBJECT_H

#include "zypp/APIConfig.h"

#include "zypp/Resolvable.h"
#include "zypp/Vendor.h"

#include "zypp/sat/LookupAttr.h"
#include "zypp/sat/SolvableSet.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class ResObject
  /// \brief Base for resolvable objects
  ///
  /// \note \ref Resolvable is a SolvableType, which provides direct
  /// access to many of the underlying sat::Solvables properties.
  /// Don't add common properties here, but in \ref sat::Solvable
  /// and extend \ref sat::SolvableType.
  ///
  /// \see \ref makeResObject for how to construct ResObjects.
  /// \todo Merge with Resolvable
  ///////////////////////////////////////////////////////////////////
  class ResObject : public Resolvable
  {
  public:
    typedef ResObject                Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /** Convert \c this into a Ptr of a certain Kind.
     * This is a convenience to access type specific
     * attributes.
     * \return \c NULL if \c this is not of the specified kind.
     * \code
     *  PoolItem pi;
     *  Package::constPtr pkg = pi->asKind<Package>();
     *
     *  if ( pi->isKind<Package>() )
     *     DBG << pi->asKind<Package>()->keywords() << endl;
     * \endcode
     */
    template<class _Res>
    inline typename ResTraits<_Res>::constPtrType asKind() const;

    template<class _Res>
    inline typename ResTraits<_Res>::PtrType asKind();

  public:
    /**
     * \short Vendor
     * \deprecated Though typedef'ed to std::string, Vendor is actually an \ref IdString.
     */
    Vendor vendor() const
    { return Resolvable::vendor().asString(); }

  protected:
    friend ResObject::Ptr makeResObject( const sat::Solvable & solvable_r );
    /** Ctor */
    ResObject( const sat::Solvable & solvable_r );
    /** Dtor */
    virtual ~ResObject();
    /** Helper for stream output */
    virtual std::ostream & dumpOn( std::ostream & str ) const;
    /** This is a \ref sat::SolvableType (allow implicit conversion in derived classes). */
    operator sat::Solvable() const
    { return satSolvable(); }
  };
  ///////////////////////////////////////////////////////////////////

  /** Create \ref ResObject from \ref sat::Solvable.
   *
   * This function creates the apropriate kind of ResObject
   * depending on the sat::Solvables kind, and returns a smart
   * pointer to it.
   *
   * If the sat::Solvables kind is not convertible, a NULL
   * pointer is returned.
   *
   * \code
   * sat::Solvable s;
   * ResObject::Ptr p( makeResObject( s ) );
   * ResObject::Ptr q( make<ResObject>( s ) );
   * Package::Ptr   pkg( make<Package>( s ) );
   * \endcode
  */
  ResObject::Ptr makeResObject( const sat::Solvable & solvable_r );

  /** Directly create a certain kind of ResObject from \ref sat::Solvable.
   *
   * If the sat::Solvables kind is not appropriate, a NULL
   * pointer is returned.
    * \code
   * sat::Solvable s;
   * ResObject::Ptr p( makeResObject( s ) );
   * ResObject::Ptr q( make<ResObject>( s ) );
   * Package::Ptr   pkg( make<Package>( s ) );
   * \endcode
   * \todo make<> was a poor choice (AFAIR because gcc had some trouble with
   * asKind<>(sat::Solvable)). Remove it in favour of asKind<>(sat::Solvable)
  */
  template<class _Res>
  inline typename ResTraits<_Res>::PtrType make( const sat::Solvable & solvable_r )
  { return( isKind<_Res>( solvable_r ) ? new _Res( solvable_r ) : 0 ); }
  /** \overload Specialisation for ResObject autodetecting the kind of resolvable. */
  template<>
  inline ResObject::Ptr make<ResObject>( const sat::Solvable & solvable_r )
  { return makeResObject( solvable_r ); }

  /** Directly create a certain kind of ResObject from \ref sat::Solvable. */
  template<class _Res>
  inline typename ResTraits<_Res>::PtrType asKind( const sat::Solvable & solvable_r )
  { return make<_Res>( solvable_r ); }

  /** Convert ResObject::Ptr into Ptr of a certain Kind.
   * \return \c NULL iff \a p is \c NULL or points to a Resolvable
   * not of the specified Kind.
   * \relates ResObject
   * \code
   * asKind<Package>(resPtr);
   * \endcode
  */
  template<class _Res>
  inline typename ResTraits<_Res>::PtrType asKind( const ResObject::Ptr & p )
  { return dynamic_pointer_cast<_Res>(p); }

  template<class _Res>
  inline typename ResTraits<_Res>::constPtrType asKind( const ResObject::constPtr & p )
  { return dynamic_pointer_cast<const _Res>(p); }

  template<class _Res>
  inline typename ResTraits<_Res>::constPtrType ResObject::asKind() const
  { return dynamic_cast<const _Res *>( this ); }

  template<class _Res>
  inline typename ResTraits<_Res>::PtrType ResObject::asKind()
  { return dynamic_cast<_Res *>( this ); }

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOBJECT_H
