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
  /**
   * Interface base for resolvable objects (common data).
   * That is, all data not needed for solving, but common
   * across al Resolvable kinds.
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
    /**
     * \short Short text describing the resolvable.
     * This attribute is usually displayed in columns.
     */
    Text summary() const;

    /**
     * \short Long text describing the resolvable.
     */
    Text description() const;

    /**
     * \short Installation Notification
     *
     * This text can be used to tell the user some notes
     * When he selects the resovable for installation.
     */
    Text insnotify() const;

    /**
     * \short De-Installation Notification
     *
     * This text can be used to tell the user some notes
     * When he selects the resovable for deinstall.
     */
    Text delnotify() const;

    /**
     * \short License or agreement to accept
     *
     * Agreement, warning or license the user should
     * accept before installing the resolvable.
     */
    Text licenseToConfirm() const;

    /**
     * \short Vendor
     *
     * For Example "Novell Inc."
     */
    Vendor vendor() const;

    /** Installed size. */
    ByteCount size() const;

    /** Size of the rpm package. */
    ByteCount archivesize() const;

    /**
     * Source providing this resolvable
     */
    Source_Ref source() const;

    /**
     * Media number where the resolvable is located
     * 0 if no media access is required.
    */
    unsigned sourceMediaNr() const;

    /**
     * \deprecated Use sourceMediaNr 
     */
    ZYPP_DEPRECATED unsigned mediaId() const
    { return sourceMediaNr(); }

    /**
     * \TODO FIXME what is this?
     */
    bool installOnly() const;

    /**
     * \short build time of the resolvable
     */
    Date buildtime() const;

    /**
     * \short Installation time
     * 0 if the resolvable is not installed.
     */
    Date installtime() const;

    /**
     * \deprecated No replacement.
     */
    ZYPP_DEPRECATED ZmdId zmdid () const;

  protected:
    /** Ctor */
    ResObject( const Kind & kind_r,
               const NVRAD & nvrad_r );
    /** Dtor */
    virtual ~ResObject();

    /** Helper for stream output */
    virtual std::ostream & dumpOn( std::ostream & str ) const;

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
