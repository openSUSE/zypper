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

#include "zypp/base/Deprecated.h"

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

      /** The autodetected system architecture. */
      static Arch defaultSystemArchitecture();

      /** The system architecture zypp uses. */
      Arch systemArchitecture() const;

      /** Override the zypp system architecture.
       * This is useful for test scenarious. <b>But be warned</b>, zypp does
       * not expect the system architecture to change at runtime. So
       * set it at the verry beginning before you acess any other
       * zypp component.
      */
      void setSystemArchitecture( const Arch & arch_r );

      /** Reset the zypp system architecture to the default. */
      void resetSystemArchitecture()
      { setSystemArchitecture( defaultSystemArchitecture() ); }

    public:
      /** The autodetected prefered locale for translated texts.
       */
      static Locale defaultTextLocale();

      /** The locale for translated texts zypp uses.
       */
      Locale textLocale() const;

      /** Set the prefered locale for translated texts. */
      void setTextLocale( const Locale & locale_r );

      /** Reset the locale for translated texts to the default. */
      void resetTextLocale()
      { setTextLocale( defaultTextLocale() ); }

    public:
     /**
       * Path where the repo metadata is downloaded and kept.
       */
      Pathname repoMetadataPath() const;

      /**
       * Path where the repo packages are downloaded and kept.
       */
      Pathname repoPackagesPath() const;

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
       * Directory for equivalent vendor definitions
       */
      Pathname vendorPath() const;

      /**
       * Solver regards required packages,patterns,... only
       */      
      bool solver_onlyRequires() const;

      /**
       * Path where zypp can find or create lock file
       */
      Pathname  locksFile() const;

      /**
       * Whetever locks file should be readed and applied after start
       */
      bool apply_locks_file() const;

    public:
      class Impl;
      /** Dtor */
      ~ZConfig();
    private:
      friend class Impl;
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
