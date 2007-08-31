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
   * It reads /etc/zypp/zypp.conf, the filename can be overridden by
   * setting the ZYPP_CONF environment variable to a different file.
   *
   * \ingroup Singleton
  */
  class ZConfig : private base::NonCopyable
  {
    public:
      /** Singleton ctor */
      static ZConfig & instance();

    public:
      /** The zypp system architecture. */
      Arch systemArchitecture() const;

      /** Override the zypp system architecture, useful for test scenarious.
	  This should be used for testing/debugging only since the Target backend
	  won't be able to install incompatible packages !!
          DONT use for normal application unless you know what you're doing. */
      void overrideSystemArchitecture( const Arch & );

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

      /** Whether to consider using a patchrpm when downloading a package.
       * Config option <tt>download.use_patchrpm (true)</tt>
      */
      bool download_use_patchrpm() const;

      /** Whether to consider using a deltarpm when downloading a package.
       * Config option <tt>download.use_deltarpm (true)</tt>
       */
      bool download_use_deltarpm() const;

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
