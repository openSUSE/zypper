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

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/ResTraits.h"

#include "zypp/Edition.h"
#include "zypp/Arch.h"
#include "zypp/CapSetFwd.h"
#include "zypp/Dep.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  struct NVRAD;
  class Dependencies;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Resolvable
  //
  /** Interface base for resolvable objects (identification and dependencies).
   * \invariant \c provides <tt>name = edition</tt>
   * \invariant \c prerequires is a subset of \c requires
  */
  class Resolvable : public base::ReferenceCounted, private base::NonCopyable
  {
  public:
    typedef Resolvable               Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::KindType     Kind;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /**  */
    const Kind & kind() const;
    /**  */
    const std::string & name() const;
    /**  */
    const Edition & edition() const;
    /**  */
    const Arch & arch() const;

    /** \name Dependencies. */
    //@{
    /** Select by Dep. */
    const CapSet & dep( Dep which_r ) const;
    /** All dependencies. */
    const Dependencies & deps() const;
    //@}

    /** \name Deprecated. */
    //@{
    void injectProvides( const Capability & cap_r );
    void injectRequires( const Capability & cap_r );
    //@}

  protected:
    /** Ctor */
    Resolvable( const Kind & kind_r,
                const NVRAD & nvrad_r );
    /** Dtor */
    virtual ~Resolvable();
    /** Helper for stream output */
    virtual std::ostream & dumpOn( std::ostream & str ) const;

  private:
    /** Implementation */
    struct Impl;
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

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
    { return p && p->kind() == ResTraits<_Res>::kind; }

  // Specialization for Resolvable: Always true.
  template<>
    inline bool isKind<Resolvable>( const Resolvable::constPtr & p )
    { return p; }

  // Specialization for ResObject: Always true.
  template<>
    inline bool isKind<ResObject>( const Resolvable::constPtr & p )
    { return p; }


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

  /** \relates Resolvable Compare Resolvable::constPtr according to
   *  \a kind and \a name.
  */
  inline int compareByN( const Resolvable::constPtr & lhs,
                         const Resolvable::constPtr & rhs )
  {
    if ( lhs == rhs )
      return 0;
    if ( ! (lhs && rhs) )
      return lhs ? 1 : -1;
    int res = 0;
    if ( (res = lhs->kind().compare( rhs->kind() )) )
      return res;
    return lhs->name().compare( rhs->name() );
  }

  /** \relates Resolvable Compare Resolvable::constPtr according to
   *  \a kind, \a name and \a edition(compare!).
  */
  inline int compareByNVR( const Resolvable::constPtr & lhs,
                           const Resolvable::constPtr & rhs )
  {
    if ( lhs == rhs )
      return 0;
    if ( ! (lhs && rhs) )
      return lhs ? 1 : -1;
    int res = 0;
    if ( (res = lhs->kind().compare( rhs->kind() )) )
      return res;
    if ( (res = lhs->name().compare( rhs->name() )) )
      return res;
    return lhs->edition().compare( rhs->edition() );
  }

  /** \relates Resolvable Compare Resolvable::constPtr according to
   *  \a kind, \a name, \a edition(compare!) and \a arch.
  */
  inline int compareByNVRA( const Resolvable::constPtr & lhs,
                            const Resolvable::constPtr & rhs )
  {
    if ( lhs == rhs )
      return 0;
    if ( ! (lhs && rhs) )
      return lhs ? 1 : -1;
    int res = 0;
    if ( (res = lhs->kind().compare( rhs->kind() )) )
      return res;
    if ( (res = lhs->name().compare( rhs->name() )) )
      return res;
    if ( (res = lhs->edition().compare( rhs->edition() )) )
      return res;
    return lhs->arch().compare( rhs->arch() );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOLVABLE_H
