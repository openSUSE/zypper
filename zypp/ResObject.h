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

#include "zypp/Resolvable.h"
#include "zypp/NeedAType.h"
#include "zypp/Date.h"
#include "zypp/ByteCount.h"
#include "zypp/DiskUsage.h"
#include "zypp/Repository.h"
#include "zypp/TranslatedText.h"
#include "zypp/OnMediaLocation.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResObject
  //
  /**
   * Interface base for resolvable objects (common data).
   * That is, all data not needed for solving, but common
   * across all Resolvable kinds.
   *
   * \see \ref makeResObject for how to construct ResObjects.
  */
  class ResObject : public Resolvable
  {
  public:
    typedef ResObject                Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /** \name Locale support.
     * \see \ref sat::Solvable
    */
    //@{
    /** Whether this \c Solvable claims to support locales. */
    using sat::Solvable::supportsLocales;
    /** Whether this \c Solvable supports a specific \ref Locale. */
    using sat::Solvable::supportsLocale;
    /** Whether this \c Solvable supports at least one of the specified locales. */
    using sat::Solvable::supportsLocale;
    /** Whether this \c Solvable supports at least one requested locale.
     * \see \ref Pool::setRequestedLocales
     */
    using sat::Solvable::supportsRequestedLocales;
    /** Return the supported locales via locales_r. */
    using sat::Solvable::getSupportedLocales;
    //@}

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
    Vendor vendor() const
    { return Resolvable::vendor().asString(); }

    /** Installed size. */
    ByteCount size() const;

    /** Size of the rpm package. */
    ByteCount downloadSize() const;

     /** \ref RepoInfo associated with the repository
      *  providing this resolvable.
      */
    RepoInfo repoInfo() const;

    /** \deprecated \ref repoInfo is provided directly.
     */
    ZYPP_DEPRECATED Repository repository() const
    { return Repository( repoInfo() ); }

    /**
     * Media number where the resolvable is located
     * 0 if no media access is required.
     */
    unsigned mediaNr() const;

    /**
     * \TODO FIXME what is this?
     * Flag in the metadata indicating this should be
     * installed unsing '-i' (not -U).
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
     * \short Disk usage per directory
     * A common attribute, although mostly packages require
     * noticeable disk space. An e.g product could try to reserve
     * a certain ammount of diskspace by providing DiskUsage data.
     */
    const DiskUsage & diskusage() const;

  protected:
    /** Ctor */
    ResObject( const sat::Solvable & solvable_r );
    /** Dtor */
    virtual ~ResObject();
    /** Helper for stream output */
    virtual std::ostream & dumpOn( std::ostream & str ) const;
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
  */
  template<class _Res>
  inline typename ResTraits<_Res>::PtrType make( const sat::Solvable & solvable_r )
  { return( isKind<_Res>( solvable_r ) ? new _Res( solvable_r ) : 0 ); }
  /** \overload Specialisation for ResObject autodetecting the kind of resolvable. */
  template<>
  inline ResObject::Ptr make<ResObject>( const sat::Solvable & solvable_r )
  { return makeResObject( solvable_r ); }

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
