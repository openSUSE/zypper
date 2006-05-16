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

#include "zypp/base/Deprecated.h"

#include "zypp/detail/ResObjectImplIf.h"
#include "zypp/Resolvable.h"
#include "zypp/TranslatedText.h"
#include "zypp/NeedAType.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace detail {
    class ImplConnect;
    class ResObjectImplIf;
  }
  class Source_Ref;
  class ByteCount;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResObject
  //
  /** Interface base for resolvable objects (common data).
  */
  class ResObject : public Resolvable
  {
  public:
    typedef detail::ResObjectImplIf  Impl;
    typedef ResObject                Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /** */
    Text summary() const;

    /** */
    Text description() const;

    /** */
    Text insnotify() const;

    /** */
    Text delnotify() const;

    /** */
    Text licenseToConfirm() const;

    /** */
    Vendor vendor() const;

    /** */
    ByteCount size() const;

    /** */
    ByteCount archivesize() const;

    /** Backlink to the source providing this. */
    Source_Ref source() const;

    /** Number of the source media that provides the data
     *  required for installation. Zero, if no media access
     *  is required.
    */
    unsigned sourceMediaNr() const;

    /** */
    bool installOnly() const;

    /** */
    Date buildtime() const;

    /** Time of installation, or \c 0 */
    Date installtime() const;

    /** */
    ZmdId zmdid () const;

  protected:
    /** Ctor */
    ResObject( const Kind & kind_r,
               const NVRAD & nvrad_r );
    /** Dtor */
    virtual ~ResObject();

  private:
    friend class detail::ImplConnect;
    /** Access implementation */
    virtual Impl & pimpl() = 0;
    /** Access implementation */
    virtual const Impl & pimpl() const = 0;
  };
  ///////////////////////////////////////////////////////////////////

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

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOBJECT_H
