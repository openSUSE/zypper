/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZConfig.h
 *
*/
#ifndef ZYPP_ZCONFIG_H
#define ZYPP_ZCONFIG_H

#include <iosfwd>

#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/Arch.h"
#include "zypp/Locale.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZConfig
  //
  /** Interim helper class to collect global options and settings.
   * Use it to avoid hardcoded values and calls to getZYpp() just
   * to retrieve some value like architecture, languages or tmppath.
   *
   * \ingroup Singleton
  */
  class ZConfig : private base::NonCopyable
  {
    public:
      /** Singleton ctor */
      static ZConfig & instance();

    public:
      /** The system architecture. */
      Arch systemArchitecture() const;

      /** The prefered locale for translated labels, descriptions,
       *  descriptions, etc. passed to the UI.
       */
      Locale textLocale() const;

      /**
       * Path where the repo metadata is downloaded and kept.
       */
      Pathname repoMetadataPath() const;

      /**
       * Path where the processed cache is kept
       * (this is where zypp.db is located.
       */
      Pathname repoCachePath() const;

      /**
       * Path where the known repositories
       * .repo files are kept
       */
      Pathname knownReposPath() const;

      /**
       * Separator string for storing/reading sets of strings to/from
       * metadata cache DB.
       */
      const std::string & cacheDBSplitJoinSeparator() const;

      /**
       * Whether repository urls should be probed.
       / config option
       * repo.add.probe
       */
      bool repo_add_probe() const;

      /**
       * Amount of time in minutes that must pass before another refresh.
       */
      unsigned repo_refresh_delay() const;

      /**
       * Whether untrusted vendor should be autolocked
       / config option
       * repo.add.probe
       */
      bool autolock_untrustedvendor() const;
      
    public:
      class Impl;
      /** Dtor */
      ~ZConfig();
    private:
      /** Default ctor. */
      ZConfig();
      /** Pointer to implementation */
      RW_pointer<Impl, rw_pointer::Scoped<Impl> > _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ZCONFIG_H
