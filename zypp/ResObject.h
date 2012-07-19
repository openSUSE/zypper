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
#include "zypp/Date.h"
#include "zypp/Locale.h"
#include "zypp/Vendor.h"
#include "zypp/ByteCount.h"
#include "zypp/DiskUsage.h"
#include "zypp/OnMediaLocation.h"
#include "zypp/Repository.h"

#include "zypp/sat/LookupAttr.h"
#include "zypp/sat/SolvableSet.h"

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
    /** \name Locale support.
     * \see \ref sat::Solvable
     */
    //@{
    /** \see \ref sat::Solvable::supportsLocales */
    bool supportsLocales() const
    { return sat::Solvable::supportsLocales(); }

    /** \see \ref sat::Solvable::supportsLocale */
    bool supportsLocale( const Locale & locale_r ) const
    { return sat::Solvable::supportsLocale( locale_r ); }

    bool supportsLocale( const LocaleSet & locales_r ) const
    { return sat::Solvable::supportsLocale( locales_r ); }

    /** \see \ref sat::Solvable::supportsRequestedLocales */
    bool supportsRequestedLocales() const
    { return sat::Solvable::supportsRequestedLocales(); }

    /** \see \ref sat::Solvable::getSupportedLocales */
    LocaleSet getSupportedLocales() const
    { return sat::Solvable::getSupportedLocales(); }
    //@}

  public:
    /**
     * \short Short text describing the resolvable.
     * This attribute is usually displayed in columns.
     */
    std::string summary( const Locale & lang_r = Locale() ) const;

    /**
     * \short Long text describing the resolvable.
     */
    std::string description( const Locale & lang_r = Locale() ) const;

    /**
     * \short Installation Notification
     *
     * This text can be used to tell the user some notes
     * When he selects the resovable for installation.
     */
    std::string insnotify( const Locale & lang_r = Locale() ) const;

    /**
     * \short De-Installation Notification
     *
     * This text can be used to tell the user some notes
     * When he selects the resovable for deinstall.
     */
    std::string delnotify( const Locale & lang_r = Locale() ) const;

    /**
     * \short License or agreement to accept
     *
     * Agreement, warning or license the user should
     * accept before installing the resolvable.
     */
    std::string licenseToConfirm( const Locale & lang_r = Locale() ) const;

    /**
     * \short Vendor
     *
     * For example "Novell Inc."
     */
    Vendor vendor() const
    { return Resolvable::vendor().asString(); }

    /** The distribution string.
     * E.g. \c code-11.
    */
    std::string distribution() const;

    /**
     * The Common Platform Enumeration name
     * for this product.
     *
     * See http://cpe.mitre.org
     */
    std::string cpeId() const;

    /** Installed size. */
    ByteCount installSize() const;

    /** Size of the rpm package. */
    ByteCount downloadSize() const;

    /** \see \ref sat::Solvable::repository */
    Repository repository() const
    { return sat::Solvable::repository(); }

     /** \ref RepoInfo associated with the repository
      *  providing this resolvable.
      */
    RepoInfo repoInfo() const
    { return repository().info(); }

    /**
     * Media number where the resolvable is located
     * 0 if no media access is required.
     */
    unsigned mediaNr() const;

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
    friend ResObject::Ptr makeResObject( const sat::Solvable & solvable_r );
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
  { return make<_Res>( *this ); }

  template<class _Res>
  inline typename ResTraits<_Res>::PtrType ResObject::asKind()
  { return make<_Res>( *this ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOBJECT_H
