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
#include <set>
#include <string>

#include "zypp/APIConfig.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/Arch.h"
#include "zypp/Locale.h"
#include "zypp/Pathname.h"
#include "zypp/IdString.h"

#include "zypp/DownloadMode.h"
#include "zypp/target/rpm/RpmFlags.h"

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
   * Note, if you add settings to this file, please follow the following
   * convention:
   *
   * namespace.settingname
   *
   * should become
   *
   * namespace_settingName()
   *
   * \ingroup ZyppConfig
   * \ingroup Singleton
  */
  class ZConfig : private base::NonCopyable
  {
    public:

      /** Singleton ctor */
      static ZConfig & instance();

      /** Print some detail about the current libzypp version.*/
      std::ostream & about( std::ostream & str ) const;

    public:

      /** The target root directory.
       * Returns an empty path if no target is set.
       */
      Pathname systemRoot() const;

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
      /** \name Maintain user data
       * \see \ref zypp-userdata
       */
      //@{
      /** Whether a (non empty) user data sting is defined. */
      bool hasUserData() const;

      /** User defined string value to be passed to log, history, plugins... */
      std::string userData() const;

      /** Set a new \ref userData string.
       * \returns \c TRUE if the string was accepted; \c FALSE if the
       * string was rejected due to nonprintable characters or newlines.
       */
      bool setUserData( const std::string & str_r );
      //@}

    public:
      /**
       * Path where the caches are kept (/var/cache/zypp)
       * \ingroup g_ZC_REPOCACHE
       */
      Pathname repoCachePath() const;

     /**
       * Path where the repo metadata is downloaded and kept (repoCachePath()/raw).
        * \ingroup g_ZC_REPOCACHE
      */
      Pathname repoMetadataPath() const;

     /**
       * Path where the repo solv files are created and kept (repoCachePath()/solv).
        * \ingroup g_ZC_REPOCACHE
      */
      Pathname repoSolvfilesPath() const;

      /**
       * Path where the repo packages are downloaded and kept (repoCachePath()/packages).
        * \ingroup g_ZC_REPOCACHE
      */
      Pathname repoPackagesPath() const;

      /**
       * Path where the configfiles are kept (/etc/zypp).
       * \ingroup g_ZC_CONFIGFILES
       */
      Pathname configPath() const;

      /**
       * Path where the known repositories .repo files are kept (configPath()/repos.d).
       * \ingroup g_ZC_CONFIGFILES
       */
      Pathname knownReposPath() const;

      /**
       * Path where the known services .service files are kept (configPath()/services.d).
       * \ingroup g_ZC_CONFIGFILES
       */
      Pathname knownServicesPath() const;

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
       * List of locales for which translated package descriptions should be downloaded.
       */
      LocaleSet repoRefreshLocales() const;

      /**
       * Whether to use repository alias or name in user messages (progress,
       * exceptions, ...).
       * True: use alias, false: use name.
       */
      bool repoLabelIsAlias() const;

      /**
       * Whether to use repository alias or name in user messages (progress,
       * exceptions, ...). Console applications might prefer to use and display
       * the shorter alias instead of full repository name.
       *
       * Default: false; i.e. repo label is 'name'
       */
      void repoLabelIsAlias( bool yesno_r );

      /**
       * Maximum number of concurrent connections for a single transfer
       */
      long download_max_concurrent_connections() const;

      /**
       * Minimum download speed (bytes per second)
       * until the connection is dropped
       */
      long download_min_download_speed() const;

      /**
       * Maximum download speed (bytes per second)
       */
      long download_max_download_speed() const;

      /**
       * Maximum silent tries
       */
      long download_max_silent_tries() const;

      /**
       * Maximum time in seconds that you allow a transfer operation to take.
       */
      long download_transfer_timeout() const;


      /** Whether to consider using a deltarpm when downloading a package.
       * Config option <tt>download.use_deltarpm (true)</tt>
       */
      bool download_use_deltarpm() const;

      /** Whether to consider using a deltarpm even when rpm is local.
       * This requires \ref download_use_deltarpm being \c true.
       * Config option <tt>download.use_deltarpm.always (false)</tt>
       */
      bool download_use_deltarpm_always() const;

      /**
       * Hint which media to prefer when installing packages (download vs. CD).
       * \see class \ref media::MediaPriority
       */
      bool download_media_prefer_download() const;
      /** \overload */
      bool download_media_prefer_volatile() const
      { return ! download_media_prefer_download(); }
      /**
       * Set \ref download_media_prefer_download to a specific value.
       */
      void set_download_media_prefer_download( bool yesno_r );
      /**
       * Set \ref download_media_prefer_download to the configfiles default.
       */
      void set_default_download_media_prefer_download();

      /**
       * Commit download policy to use as default.
       */
      DownloadMode commit_downloadMode() const;

      /**
       * Directory for equivalent vendor definitions  (configPath()/vendors.d)
       * \ingroup g_ZC_CONFIGFILES
       */
      Pathname vendorPath() const;

      /**
       * Solver regards required packages,patterns,... only
       */
      bool solver_onlyRequires() const;

      /**
       * File in which dependencies described which has to be
       * fulfilled for a running system.
       */
      Pathname solver_checkSystemFile() const;

      /**
       * Whether vendor check is by default enabled.
       */
      bool solver_allowVendorChange() const;

      /**
       * Whether removing a package should also remove no longer needed requirements.
       */
      bool solver_cleandepsOnRemove() const;

      /**
       * When committing a dist upgrade (e.g. <tt>zypper dup</tt>)
       * a solver testcase is written. It is needed in bugreports,
       * in case something went wrong. This returns the number of
       * testcases to keep on the system. Old cases will be deleted,
       * as new ones are created. Use \c 0 to write no testcase at all.
       */
      unsigned solver_upgradeTestcasesToKeep() const;

      /** Whether dist upgrade should remove a products dropped packages (true).
       *
       * A new product may suggest a list of old and no longer supported
       * packages (dropped packages). Performing a dist upgrade the solver
       * may try to delete them, even if they do not cause any dependency
       * problem.
       *
       * Turning this option off, the solver will not try to remove those
       * packages unless they actually do cause dependency trouble. At any
       * time you may use zypper to detect orphaned packages, and do the
       * cleanup manually. Or simply leave them installed as long as you don't
       * need the disk space.
       */
      bool solverUpgradeRemoveDroppedPackages() const;
      /** Set \ref solverUpgradeRemoveDroppedPackages to \a val_r. */
      void setSolverUpgradeRemoveDroppedPackages( bool val_r );
      /** Reset \ref solverUpgradeRemoveDroppedPackages to the \c zypp.conf default. */
      void resetSolverUpgradeRemoveDroppedPackages();

      /** \name Packages which can be installed in different versions at the same time.
       * This returns the config file values (\c names or \c provides:...). For the corresponding
       * packages use e.g \ref sat::Pool::multiversionBegin, or \ref sat::Solbale::multiversionInstall
       * (\ref ui::Selectable::multiversionInstall).
       */
      //@{
      const std::set<std::string> & multiversionSpec() const;
      void addMultiversionSpec( const std::string & name_r );
      void removeMultiversionSpec( const std::string & name_r );
      //@}

      /**
       * Path where zypp can find or create lock file (configPath()/locks)
       * \ingroup g_ZC_CONFIGFILES
       */
      Pathname locksFile() const;

      /**
       * Whether locks file should be read and applied after start (true)
       */
      bool apply_locks_file() const;

      /**
       * Path where the update items are kept (/var/adm)
       */
      Pathname update_dataPath() const;

     /**
      * Path where the repo metadata is downloaded and kept (update_dataPath()/).
      * \ingroup g_ZC_REPOCACHE
      */
      Pathname update_scriptsPath() const;

     /**
      * Path where the repo solv files are created and kept (update_dataPath()/solv).
      * \ingroup g_ZC_REPOCACHE
      */
      Pathname update_messagesPath() const;

      /** \name Command to be invoked to send update messages. */
      //@{
      /** Command definition for sending update messages.*/
      std::string updateMessagesNotify() const;
      /** Set a new command definition (see update.messages.notify in zypp.conf). */
      void setUpdateMessagesNotify( const std::string & val_r );
      /** Reset to the zypp.conf default. */
      void resetUpdateMessagesNotify();
     //@}

      /** \name Options for package installation */
      //@{
      /** The default \ref target::rpm::RpmInstFlags for \ref ZYppCommitPolicy.
       * Or-combination of \ref target::rpm::RpmInstFlag.
       * \code
       * ZConfig.instance().rpmInstallFlags().testFlag( target::rpm::RPMINST_EXCLUDEDOCS );
       * \endcode
       */
      target::rpm::RpmInstFlags rpmInstallFlags() const;
      //@}

      /**
       * Path where ZYpp install history is logged. Defaults to
       * /var/log/zypp/history.
       *
       * \see http://en.opensuse.org/Libzypp/Package_History
       */
      Pathname historyLogFile() const;

      /**
       * Defaults to /etc/zypp/credentials.d
       */
      Pathname credentialsGlobalDir() const;

      /**
       * Defaults to /etc/zypp/credentials.cat
       */
      Pathname credentialsGlobalFile() const;

      /** Package telling the "product version" on systems not using /etc/product.d/baseproduct.
       *
       * On RHEL, Fedora and others the "product version" is determined by the first package
       * providing 'redhat-release'. This value is not hardcoded in YUM and can be configured
       * with the $distroverpkg variable.
       *
       * Defaults to 'redhat-release'.
       */
      std::string distroverpkg() const;

      /** \name Plugins */
      //@{
      /**
       * Defaults to \c /usr/lib/zypp/plugins
       */
      Pathname pluginsPath() const;

      //@}
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
