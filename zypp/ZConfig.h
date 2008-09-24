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

#include "zypp/base/Deprecated.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/Arch.h"
#include "zypp/Locale.h"
#include "zypp/Pathname.h"
#include "zypp/IdString.h"

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

      /** Whether to consider using a deltarpm when downloading a package.
       * Config option <tt>download.use_deltarpm (true)</tt>
       */
      bool download_use_deltarpm() const;

      /**
       * Directory for equivalent vendor definitions  (configPath()/vendors.d)
       * \ingroup g_ZC_CONFIGFILES
       */
      Pathname vendorPath() const;

      /**
       * Directory for additional product information  (configPath()/products.d)
       * \ingroup g_ZC_CONFIGFILES
       */
      Pathname productsPath() const;

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
       * Packages which can be installed parallel with different versions
       * Returning a set of package names (IdString)
       */
      std::set<IdString> multiversion() const;
      void addMultiversion(std::string &name);
      bool removeMultiversion(std::string &name);

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
