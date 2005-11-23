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
#include "zypp/Dependencies.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Resolvable
  //
  /** Interface base for resolvable objects (identification and dependencies).
  */
  class Resolvable : public base::ReferenceCounted, private base::NonCopyable
  {
  public:
    typedef Resolvable               Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::KindType     Kind;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;
    friend std::ostream & operator<<( std::ostream & str, const Resolvable & obj );

  public:
    /**  */
    const Kind & kind() const;
    /**  */
    const std::string & name() const;
    /**  */
    const Edition & edition() const;
    /**  */
    const Arch & arch() const;
    /**  */
    const Dependencies & deps() const;
    /** */
    void setDeps( const Dependencies & val_r );

  protected:
    /** Ctor */
    Resolvable( const Kind & kind_r,
                const std::string & name_r,
                const Edition & edition_r,
                const Arch & arch_r );
    /** Dtor */
    virtual ~Resolvable();
    /** Helper for stream output */
    virtual std::ostream & dumpOn( std::ostream & str ) const;

  private:
    /** Implementation */
    struct Impl;
    /** Pointer to implementation */
    base::RW_pointer<Impl> _pimpl;
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
    { return p && p->kind() == ResTraits<_Res>::_kind; }

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
    { return base::dynamic_pointer_cast<_Res>(p); }

  template<class _Res>
    inline typename ResTraits<_Res>::constPtrType asKind( const Resolvable::constPtr & p )
    { return base::dynamic_pointer_cast<const _Res>(p); }

  ///////////////////////////////////////////////////////////////////

  /** Required by base::intrusive_ptr to add a reference. */
  inline void intrusive_ptr_add_ref( const Resolvable * ptr_r )
  { base::ReferenceCounted::add_ref( ptr_r ); }

  /** Required by base::intrusive_ptr to release a reference. */
  inline void intrusive_ptr_release( const Resolvable * ptr_r )
  { base::ReferenceCounted::release( ptr_r ); }

  /** \relates Resolvable Stream output via Resolvable::dumpOn */
  inline std::ostream & operator<<( std::ostream & str, const Resolvable & obj )
  { return obj.dumpOn( str ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOLVABLE_H
