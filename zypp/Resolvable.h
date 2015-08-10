/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Resolvable.h
 *
*/
#ifndef ZYPP_RESOLVABLE_H
#define ZYPP_RESOLVABLE_H

#include <iosfwd>
#include <string>

#include "zypp/APIConfig.h"

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/sat/SolvableType.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  class PoolItem;
  ///////////////////////////////////////////////////////////////////
  /// \class Resolvable
  /// \brief Base for resolvable objects
  ///
  /// \note \ref Resolvable is a SolvableType, which provides direct
  /// access to many of the underlying sat::Solvables properties.
  /// Don't add common properties here, but in \ref sat::Solvable
  /// and extend \ref sat::SolvableType.
  ///
  /// In most cases you want to retrieve the common properties directly
  /// from a \ref PoolItem or \ref sat::Solvable. Construction from and
  /// explicit conversion to sat::Solvable are supported. Next goal is
  /// to get rid of the smart pointer hierarchy. A Resolvable is actually
  /// an unsigned and derived classes contain no data, so it makes little
  /// sense to wrap this into ReferenceCounted smart pointer.
  ///
  /// \todo Merge with ResObject
  /// \todo Get rid of refcout/smart_prt bloat, as this type is actually IdBased (i.e. sizeof(unsigned))
  ///////////////////////////////////////////////////////////////////
  class Resolvable : public sat::SolvableType<Resolvable>,
                     public base::ReferenceCounted, private base::NonCopyable
  {
    friend std::ostream & operator<<( std::ostream & str, const Resolvable & obj );

  public:
    typedef Resolvable               Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::KindType     Kind;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /** This is a \ref sat::SolvableType. */
    explicit operator sat::Solvable() const
    { return _solvable; }

    /** Access the corresponding \ref PoolItem. */
    PoolItem poolItem() const;

  protected:
    /** Ctor */
    Resolvable( const sat::Solvable & solvable_r );
    /** Dtor */
    virtual ~Resolvable();
    /** Helper for stream output */
    virtual std::ostream & dumpOn( std::ostream & str ) const;

  private:
    sat::Solvable _solvable;
 };
 ///////////////////////////////////////////////////////////////////

 /** \relates Resolvable Stream output */
 inline std::ostream & operator<<( std::ostream & str, const Resolvable & obj )
 { return obj.dumpOn( str ); }

 /** \relates Resolvable More verbose stream output including dependencies */
 inline std::ostream & dumpOn( std::ostream & str, const Resolvable & obj )
 { return dumpOn( str, obj.satSolvable() ); }

 /** Test whether a Resolvable::Ptr is of a certain Kind.
  * \return \c Ture iff \a p is not \c NULL and points to a Resolvable
  * of the specified Kind.
  * \relates Resolvable
  * \code
  * isKind<Package>(resPtr);
  * \endcode
  */
 template<class _Res>
 inline bool isKind( const Resolvable::constPtr & p )
 { return p && p->isKind<_Res>(); }

 // Specialization for Resolvable: Always true.
 template<>
 inline bool isKind<Resolvable>( const Resolvable::constPtr & p )
 { return !!p; }

 // Specialization for ResObject: Always true.
 template<>
 inline bool isKind<ResObject>( const Resolvable::constPtr & p )
 { return !!p; }


 /** Convert Resolvable::Ptr into Ptr of a certain Kind.
  * \return \c NULL iff \a p is \c NULL or points to a Resolvable
  * not of the specified Kind.
  * \relates Resolvable
  * \code
  * asKind<Package>(resPtr);
  * \endcode
  */
 template<class _Res>
 inline typename ResTraits<_Res>::PtrType asKind( const Resolvable::Ptr & p )
 { return dynamic_pointer_cast<_Res>(p); }

 template<class _Res>
 inline typename ResTraits<_Res>::constPtrType asKind( const Resolvable::constPtr & p )
 { return dynamic_pointer_cast<const _Res>(p); }

  ///////////////////////////////////////////////////////////////////

  /** \relates Resolvable Compare Resolvable::constPtr according to \a kind and \a name.
   * \deprecated Get rid of refcout/smart_prt bloat, use
   */
  inline int compareByN( const Resolvable::constPtr & lhs, const Resolvable::constPtr & rhs )
  {
    int ret = 0;
    if ( lhs != rhs )
    {
      if ( lhs && rhs )
	ret = compareByN( *lhs, *rhs );
      else
	ret = lhs ? 1 : -1;
    }
    return ret;
  }

  /** \relates Resolvable Compare according to \a kind, \a name and \a edition. */
  inline int compareByNVR( const Resolvable::constPtr & lhs, const Resolvable::constPtr & rhs )
  {
    int ret = 0;
    if ( lhs != rhs )
    {
      if ( lhs && rhs )
	ret = compareByNVR( *lhs, *rhs );
      else
	ret = lhs ? 1 : -1;
    }
    return ret;
  }

  /** \relates Resolvable Compare Resolvable::constPtr according to \a kind, \a name, \a edition and \a arch. */
  inline int compareByNVRA( const Resolvable::constPtr & lhs, const Resolvable::constPtr & rhs )
  {
    int ret = 0;
    if ( lhs != rhs )
    {
      if ( lhs && rhs )
	ret = compareByNVRA( *lhs, *rhs );
      else
	ret = lhs ? 1 : -1;
    }
    return ret;
  }
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOLVABLE_H
