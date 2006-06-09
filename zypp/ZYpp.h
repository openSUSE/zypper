/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZYpp.h
 *
*/
#ifndef ZYPP_ZYPP_H
#define ZYPP_ZYPP_H

#include <iosfwd>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/base/Deprecated.h"
#include "zypp/ZYppCommit.h"

#include "zypp/Target.h"
#include "zypp/Resolver.h"
#include "zypp/KeyRing.h"
#include "zypp/DiskUsageCounter.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace zypp_detail
  {
    class ZYppImpl;
  }

  class ZYppFactory;
  class ResPool;
  class ResPoolProxy;
  class SourceFeed_Ref;
  class ResStore;
  class Locale;
  class KeyRing;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZYpp
  //
  /**
   * \todo define Exceptions
  */
  class ZYpp : public base::ReferenceCounted, private base::NonCopyable
  {
  public:

    typedef intrusive_ptr<ZYpp>       Ptr;
    typedef intrusive_ptr<const ZYpp> constPtr;

  public:

    /** Pool of ResStatus for individual ResObjetcs. */
    ResPool pool() const;

    /** Pool of ui::Selectable.
     * Based on the ResPool, ui::Selectable groups ResObjetcs of
     * same kind and name.
    */
    ResPoolProxy poolProxy() const;

    /**  */
    //SourceFeed_Ref sourceFeed() const;

    void addResolvables (const ResStore& store, bool installed = false);

    void removeResolvables (const ResStore& store);

    DiskUsageCounter::MountPointSet diskUsage();

    void setPartitions(const DiskUsageCounter::MountPointSet &mp);

  public:
    /**
     * \throws Exception
     */
    Target_Ptr target() const;

    /**
     * \throws Exception
     * if commit_only == true, just init, don't populate store or pool
     */
    void initTarget(const Pathname & root, bool commit_only = false);

    /**
     * \throws Exception
     */
    void finishTarget();


  public:
    typedef ZYppCommitResult CommitResult;

    /** Commit changes and transactions.
     * \param \ref CommitPolicy
     * \return \ref CommitResult
     * \throws Exception
    */
    ZYppCommitResult commit( const ZYppCommitPolicy & policy_r );

  public:
    /** */
    Resolver_Ptr resolver() const;
    KeyRing_Ptr keyRing() const;
  public:
    /** Set the preferd locale for translated labels, descriptions,
     *  etc. passed to the UI.
     */
    void setTextLocale( const Locale & textLocale_r );
    /** */
    Locale getTextLocale() const;

  public:
    typedef std::set<Locale> LocaleSet;
    /** Set the requested locales.
     * Languages to be supported by the system, e.g. language specific
     * packages to be installed. This function operates on the pool,
     * so only the locales that are available as resolvables
     * are marked as requested. The rest is ignored.
    */
    void setRequestedLocales( const LocaleSet & locales_r );
    /** */
    LocaleSet getRequestedLocales() const;

    /**
     * Get the set of available locales.
     * This is computed from the package data so it actually
     * represents all locales packages claim to support.
     */
    LocaleSet getAvailableLocales() const;

    /**
     * internal use only
     **/
    void availableLocale( const Locale & locale_r );

  public:
    /** Get the path where zypp related plugins store persistent data and caches   */
    Pathname homePath() const;
    
    /** Get the path where zypp related plugins store temp data   */
    Pathname tmpPath() const;
    
    /** set the home, if you need to change it */
    void setHomePath( const Pathname & path );

    /** Get the system architecture.   */
    Arch architecture() const;
    /** Set the system architecture.
	This should be used for testing/debugging only since the Target backend
	won't be able to install incompatible packages ;-)   */
    void setArchitecture( const Arch & arch );

  protected:
    /** Dtor */
    virtual ~ZYpp();
    /** Stream output */
    virtual std::ostream & dumpOn( std::ostream & str ) const;
  private:
    /** Factory */
    friend class ZYppFactory;

    /** */
    typedef zypp_detail::ZYppImpl Impl;
    typedef shared_ptr<Impl>      Impl_Ptr;
    /** Factory ctor */
    explicit
    ZYpp( const Impl_Ptr & impl_r );
  private:
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ZYPP_H
