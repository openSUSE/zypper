/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/zypp_detail/ZYppImpl.h
 *
*/
#ifndef ZYPP_ZYPP_DETAIL_ZYPPIMPL_H
#define ZYPP_ZYPP_DETAIL_ZYPPIMPL_H

#include <iosfwd>

#include "zypp/TmpPath.h"
#include "zypp/ResPoolManager.h"
#include "zypp/SourceFeed.h"
#include "zypp/Target.h"
#include "zypp/Resolver.h"
#include "zypp/Locale.h"
#include "zypp/KeyRing.h"
#include "zypp/ZYppCommit.h"
#include "zypp/DiskUsageCounter.h"

using namespace zypp::filesystem;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace zypp_detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ZYppImpl
    //
    /** */
    class ZYppImpl
    {
      friend std::ostream & operator<<( std::ostream & str, const ZYppImpl & obj );

    public:
      /** Default ctor */
      ZYppImpl();
      /** Dtor */
      ~ZYppImpl();

    private:
      void removeInstalledResolvables ();

    public:
      /** */
      ResPool pool() const
      { return _pool.accessor(); }

      ResPoolProxy poolProxy() const
      { return _pool.proxy(); }

      /** */
      SourceFeed_Ref sourceFeed() const
      { return _sourceFeed; }

      /** */
      KeyRing_Ptr keyRing() const
      { return _keyring; }


      Resolver_Ptr resolver() const
      { return _resolver; }

      void addResolvables (const ResStore& store, bool installed = false);

      void removeResolvables (const ResStore& store);

    public:
      /** \todo Signal locale change. */
      /**
       * \throws Exception
       */
      Target_Ptr target() const;

      /**
       * \throws Exception
       * if commit_only == true, just init the target, dont populate store or pool
       */
      void initTarget(const Pathname & root, bool commit_only = false);

      /**
       * \throws Exception
       */
      void finishTarget();

      /** Commit changes and transactions. */
      ZYppCommitResult commit( const ZYppCommitPolicy & policy_r );

    public:
      /** \todo Signal locale change. */
      void setTextLocale( const Locale & textLocale_r )
      { _textLocale = textLocale_r; }
      /** */
      Locale getTextLocale() const
      { return _textLocale; }
    private:
      Locale _textLocale;

    public:
      typedef std::set<Locale> LocaleSet;
      /** */
      void setRequestedLocales( const LocaleSet & locales_r );
      /** */
      LocaleSet getRequestedLocales() const;
      /** */
      LocaleSet getAvailableLocales() const;

      /** internal use */
      void availableLocale( const Locale & locale_r );

    public:
      /** Get the system architecture.   */
      Arch architecture() const
      { return _architecture; }
      /** Set the system architecture.
	  This should be used for testing/debugging only since the Target backend
	  won't be able to install incompatible packages ;-)   */
      void setArchitecture( const Arch & arch );

      /** Get the path where zypp related plugins store persistent data and caches   */
      const Pathname homePath() const;
      
      /** Get the path where zypp related plugins store tmp data   */
      const Pathname tmpPath() const;
      
      /** set the home, if you need to change it */
      void setHomePath( const Pathname & path );

    public:
      DiskUsageCounter::MountPointSet diskUsage();
      void setPartitions(const DiskUsageCounter::MountPointSet &mp);

    private:
      /** */
      ResPoolManager _pool;
      /** */
      SourceFeed_Ref _sourceFeed;
      /** */
      Target_Ptr _target;
      /** */
      Resolver_Ptr _resolver;

      KeyRing_Ptr _keyring;
      /** */
      Arch _architecture;
      /** */
      Pathname _home_path;
      /** defined mount points, used for disk usage counting */
      DiskUsageCounter _disk_usage;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates ZYppImpl Stream output */
    std::ostream & operator<<( std::ostream & str, const ZYppImpl & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace zypp_detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ZYPP_DETAIL_ZYPPIMPL_H
