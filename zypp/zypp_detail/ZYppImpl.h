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

#include "zypp/ResPoolManager.h"
#include "zypp/SourceFeed.h"
#include "zypp/Target.h"
#include "zypp/Resolver.h"

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

    public:
      /** */
      ResPool pool() const
      { return _pool.accessor(); }

      ResPoolProxy poolProxy() const
      { return _pool.proxy(); }

      /** */
      SourceFeed_Ref sourceFeed() const
      { return _sourceFeed; }

      Resolver_Ptr resolver() const
      { return _resolver; }

      void addResolvables (const ResStore& store, bool installed = false);

      void removeResolvables (const ResStore& store);

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

    public:
      /** */
      void setTextLocale( const Locale & textLocale_r )
      {}
      /** */
      Locale getTextLocale() const
      { return Locale(); }

    public:
      typedef std::set<Locale> LocaleSet;
      /** */
      void setRequestedLocales( const LocaleSet & locales_r )
      {}
      /** */
      LocaleSet getRequestedLocales() const
      { return LocaleSet(); }

    private:
      /** */
      ResPoolManager _pool;
      /** */
      SourceFeed_Ref _sourceFeed;
      /** */
      Target_Ptr _target;
      /** */
      Resolver_Ptr _resolver;
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
