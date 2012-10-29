/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/RepoManager.cc
 *
*/

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <map>
#include <algorithm>

#include "zypp/base/InputStream.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Function.h"
#include "zypp/base/Regex.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"

#include "zypp/ServiceInfo.h"
#include "zypp/repo/RepoException.h"
#include "zypp/RepoManager.h"

#include "zypp/media/MediaManager.h"
#include "zypp/media/CredentialManager.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/ExternalProgram.h"
#include "zypp/ManagedFile.h"

#include "zypp/parser/RepoFileReader.h"
#include "zypp/parser/ServiceFileReader.h"
#include "zypp/repo/ServiceRepos.h"
#include "zypp/repo/yum/Downloader.h"
#include "zypp/repo/susetags/Downloader.h"
#include "zypp/parser/plaindir/RepoParser.h"
#include "zypp/repo/PluginServices.h"

#include "zypp/Target.h" // for Target::targetDistribution() for repo index services
#include "zypp/ZYppFactory.h" // to get the Target from ZYpp instance
#include "zypp/HistoryLog.h" // to write history :O)

#include "zypp/ZYppCallbacks.h"

#include "sat/Pool.h"

using std::endl;
using std::string;
using namespace zypp::repo;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace
  {
    /** Simple media mounter to access non-downloading URLs e.g. for non-local plaindir repos.
     * \ingroup g_RAII
    */
    class MediaMounter
    {
      public:
        /** Ctor provides media access. */
        MediaMounter( const Url & url_r )
        {
          media::MediaManager mediamanager;
          _mid = mediamanager.open( url_r );
          mediamanager.attach( _mid );
        }

        /** Ctor releases the media. */
        ~MediaMounter()
        {
          media::MediaManager mediamanager;
          mediamanager.release( _mid );
          mediamanager.close( _mid );
        }

        /** Convert a path relative to the media into an absolute path.
         *
         * Called without argument it returns the path to the medias root directory.
        */
        Pathname getPathName( const Pathname & path_r = Pathname() ) const
        {
          media::MediaManager mediamanager;
          return mediamanager.localPath( _mid, path_r );
        }

      private:
        media::MediaAccessId _mid;
    };

    /** Check if alias_r is present in repo/service container. */
    template <class Iterator>
    inline bool foundAliasIn( const std::string & alias_r, Iterator begin_r, Iterator end_r )
    {
      for_( it, begin_r, end_r )
        if ( it->alias() == alias_r )
          return true;
      return false;
    }
    /** \overload */
    template <class Container>
    inline bool foundAliasIn( const std::string & alias_r, const Container & cont_r )
    { return foundAliasIn( alias_r, cont_r.begin(), cont_r.end() ); }

    /** Find alias_r in repo/service container. */
    template <class Iterator>
    inline Iterator findAlias( const std::string & alias_r, Iterator begin_r, Iterator end_r )
    {
      for_( it, begin_r, end_r )
        if ( it->alias() == alias_r )
          return it;
      return end_r;
    }
    /** \overload */
    template <class Container>
    inline typename Container::iterator findAlias( const std::string & alias_r, Container & cont_r )
    { return findAlias( alias_r, cont_r.begin(), cont_r.end() ); }
    /** \overload */
    template <class Container>
    inline typename Container::const_iterator findAlias( const std::string & alias_r, const Container & cont_r )
    { return findAlias( alias_r, cont_r.begin(), cont_r.end() ); }
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoManagerOptions
  //
  ///////////////////////////////////////////////////////////////////

  RepoManagerOptions::RepoManagerOptions( const Pathname & root_r )
  {
    repoCachePath         = Pathname::assertprefix( root_r, ZConfig::instance().repoCachePath() );
    repoRawCachePath      = Pathname::assertprefix( root_r, ZConfig::instance().repoMetadataPath() );
    repoSolvCachePath     = Pathname::assertprefix( root_r, ZConfig::instance().repoSolvfilesPath() );
    repoPackagesCachePath = Pathname::assertprefix( root_r, ZConfig::instance().repoPackagesPath() );
    knownReposPath        = Pathname::assertprefix( root_r, ZConfig::instance().knownReposPath() );
    knownServicesPath     = Pathname::assertprefix( root_r, ZConfig::instance().knownServicesPath() );
    pluginsPath           = Pathname::assertprefix( root_r, ZConfig::instance().pluginsPath() );
    probe                 = ZConfig::instance().repo_add_probe();

    rootDir = root_r;
  }

  RepoManagerOptions RepoManagerOptions::makeTestSetup( const Pathname & root_r )
  {
    RepoManagerOptions ret;
    ret.repoCachePath         = root_r;
    ret.repoRawCachePath      = root_r/"raw";
    ret.repoSolvCachePath     = root_r/"solv";
    ret.repoPackagesCachePath = root_r/"packages";
    ret.knownReposPath        = root_r/"repos.d";
    ret.knownServicesPath     = root_r/"services.d";
    ret.pluginsPath           = root_r/"plugins";
    ret.rootDir = root_r;
    return ret;
  }

  ////////////////////////////////////////////////////////////////////////////

  /**
    * \short Simple callback to collect the results
    *
    * Classes like RepoFileParser call the callback
    * once per each repo in a file.
    *
    * Passing this functor as callback, you can collect
    * all results at the end, without dealing with async
    * code.
    *
    * If targetDistro is set, all repos with non-empty RepoInfo::targetDistribution()
    * will be skipped.
    *
    * \todo do this through a separate filter
    */
    struct RepoCollector : private base::NonCopyable
    {
      RepoCollector()
      {}

      RepoCollector(const std::string & targetDistro_)
        : targetDistro(targetDistro_)
      {}

      bool collect( const RepoInfo &repo )
      {
        // skip repositories meant for other distros than specified
        if (!targetDistro.empty()
            && !repo.targetDistribution().empty()
            && repo.targetDistribution() != targetDistro)
        {
          MIL
            << "Skipping repository meant for '" << targetDistro
            << "' distribution (current distro is '"
            << repo.targetDistribution() << "')." << endl;

          return true;
        }

        repos.push_back(repo);
        return true;
      }

      RepoInfoList repos;
      std::string targetDistro;
    };

  ////////////////////////////////////////////////////////////////////////////

  /**
   * Reads RepoInfo's from a repo file.
   *
   * \param file pathname of the file to read.
   */
  static std::list<RepoInfo> repositories_in_file( const Pathname & file )
  {
    MIL << "repo file: " << file << endl;
    RepoCollector collector;
    parser::RepoFileReader parser( file, bind( &RepoCollector::collect, &collector, _1 ) );
    return collector.repos;
  }

  ////////////////////////////////////////////////////////////////////////////

  /**
   * \short List of RepoInfo's from a directory
   *
   * Goes trough every file ending with ".repo" in a directory and adds all
   * RepoInfo's contained in that file.
   *
   * \param dir pathname of the directory to read.
   */
  static std::list<RepoInfo> repositories_in_dir( const Pathname &dir )
  {
    MIL << "directory " << dir << endl;
    std::list<RepoInfo> repos;
    std::list<Pathname> entries;
    if ( filesystem::readdir( entries, dir, false ) != 0 )
    {
      // TranslatorExplanation '%s' is a pathname
      ZYPP_THROW(Exception(str::form(_("Failed to read directory '%s'"), dir.c_str())));
    }

    str::regex allowedRepoExt("^\\.repo(_[0-9]+)?$");
    for ( std::list<Pathname>::const_iterator it = entries.begin(); it != entries.end(); ++it )
    {
      if (str::regex_match(it->extension(), allowedRepoExt))
      {
        std::list<RepoInfo> tmp = repositories_in_file( *it );
        repos.insert( repos.end(), tmp.begin(), tmp.end() );

        //std::copy( collector.repos.begin(), collector.repos.end(), std::back_inserter(repos));
        //MIL << "ok" << endl;
      }
    }
    return repos;
  }

  ////////////////////////////////////////////////////////////////////////////

   std::list<RepoInfo> readRepoFile(const Url & repo_file)
   {
     // no interface to download a specific file, using workaround:
     //! \todo add MediaManager::provideFile(Url file_url) to easily access any file URLs? (no need for media access id or media_nr)
     Url url(repo_file);
     Pathname path(url.getPathName());
     url.setPathName ("/");
     MediaSetAccess access(url);
     Pathname local = access.provideFile(path);

     DBG << "reading repo file " << repo_file << ", local path: " << local << endl;

     return repositories_in_file(local);
   }

  ////////////////////////////////////////////////////////////////////////////

  inline void assert_alias( const RepoInfo & info )
  {
    if ( info.alias().empty() )
      ZYPP_THROW( RepoNoAliasException() );
    // bnc #473834. Maybe we can match the alias against a regex to define
    // and check for valid aliases
    if ( info.alias()[0] == '.')
      ZYPP_THROW(RepoInvalidAliasException(
         info, _("Repository alias cannot start with dot.")));
  }

  inline void assert_alias( const ServiceInfo & info )
  {
    if ( info.alias().empty() )
      ZYPP_THROW( ServiceNoAliasException() );
    // bnc #473834. Maybe we can match the alias against a regex to define
    // and check for valid aliases
    if ( info.alias()[0] == '.')
      ZYPP_THROW(ServiceInvalidAliasException(
         info, _("Service alias cannot start with dot.")));
  }

  ////////////////////////////////////////////////////////////////////////////

  inline void assert_urls( const RepoInfo & info )
  {
    if ( info.baseUrlsEmpty() )
      ZYPP_THROW( RepoNoUrlException( info ) );
  }

  inline void assert_url( const ServiceInfo & info )
  {
    if ( ! info.url().isValid() )
      ZYPP_THROW( ServiceNoUrlException( info ) );
  }

  ////////////////////////////////////////////////////////////////////////////

  /**
   * \short Calculates the raw cache path for a repository, this is usually
   * /var/cache/zypp/alias
   */
  inline Pathname rawcache_path_for_repoinfo( const RepoManagerOptions &opt, const RepoInfo &info )
  {
    assert_alias(info);
    return opt.repoRawCachePath / info.escaped_alias();
  }

  /**
   * \short Calculates the raw product metadata path for a repository, this is
   * inside the raw cache dir, plus an optional path where the metadata is.
   *
   * It should be different only for repositories that are not in the root of
   * the media.
   * for example /var/cache/zypp/alias/addondir
   */
  inline Pathname rawproductdata_path_for_repoinfo( const RepoManagerOptions &opt, const RepoInfo &info )
  {
    assert_alias(info);
    return opt.repoRawCachePath / info.escaped_alias() / info.path();
  }


  /**
   * \short Calculates the packages cache path for a repository
   */
  inline Pathname packagescache_path_for_repoinfo( const RepoManagerOptions &opt, const RepoInfo &info )
  {
    assert_alias(info);
    return opt.repoPackagesCachePath / info.escaped_alias();
  }

  /**
   * \short Calculates the solv cache path for a repository
   */
  inline Pathname solv_path_for_repoinfo( const RepoManagerOptions &opt, const RepoInfo &info)
  {
    assert_alias(info);
    return opt.repoSolvCachePath / info.escaped_alias();
  }

  ////////////////////////////////////////////////////////////////////////////

  /** Functor collecting ServiceInfos into a ServiceSet. */
  class ServiceCollector
  {
    public:
      typedef std::set<ServiceInfo> ServiceSet;

      ServiceCollector( ServiceSet & services_r )
      : _services( services_r )
      {}

      bool operator()( const ServiceInfo & service_r ) const
      {
        _services.insert( service_r );
        return true;
      }

    private:
      ServiceSet & _services;
  };

  ////////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoManager::Impl
  //
  ///////////////////////////////////////////////////////////////////

  /**
   * \short RepoManager implementation.
   */
  struct RepoManager::Impl
  {
    Impl( const RepoManagerOptions &opt )
      : options(opt)
    {
      init_knownServices();
      init_knownRepositories();
    }


    RepoManagerOptions options;

    RepoSet repos;

    ServiceSet services;

  public:

    void saveService( ServiceInfo & service ) const;

    Pathname generateNonExistingName( const Pathname &dir,
                                      const std::string &basefilename ) const;

    std::string generateFilename( const RepoInfo & info ) const;
    std::string generateFilename( const ServiceInfo & info ) const;


  private:
    void init_knownServices();
    void init_knownRepositories();

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };

  ///////////////////////////////////////////////////////////////////

  /** \relates RepoManager::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const RepoManager::Impl & obj )
  {
    return str << "RepoManager::Impl";
  }

  ///////////////////////////////////////////////////////////////////

  void RepoManager::Impl::saveService( ServiceInfo & service ) const
  {
    filesystem::assert_dir( options.knownServicesPath );
    Pathname servfile = generateNonExistingName( options.knownServicesPath,
                                                 generateFilename( service ) );
    service.setFilepath( servfile );

    MIL << "saving service in " << servfile << endl;

    std::ofstream file( servfile.c_str() );
    if ( !file )
    {
      // TranslatorExplanation '%s' is a filename
      ZYPP_THROW( Exception(str::form( _("Can't open file '%s' for writing."), servfile.c_str() )));
    }
    service.dumpAsIniOn( file );
    MIL << "done" << endl;
  }

  /**
   * Generate a non existing filename in a directory, using a base
   * name. For example if a directory contains 3 files
   *
   * |-- bar
   * |-- foo
   * `-- moo
   *
   * If you try to generate a unique filename for this directory,
   * based on "ruu" you will get "ruu", but if you use the base
   * "foo" you will get "foo_1"
   *
   * \param dir Directory where the file needs to be unique
   * \param basefilename string to base the filename on.
   */
  Pathname RepoManager::Impl::generateNonExistingName( const Pathname & dir,
                                                       const std::string & basefilename ) const
  {
    std::string final_filename = basefilename;
    int counter = 1;
    while ( PathInfo(dir + final_filename).isExist() )
    {
      final_filename = basefilename + "_" + str::numstring(counter);
      counter++;
    }
    return dir + Pathname(final_filename);
  }

  ////////////////////////////////////////////////////////////////////////////

  /**
   * \short Generate a related filename from a repo info
   *
   * From a repo info, it will try to use the alias as a filename
   * escaping it if necessary. Other fallbacks can be added to
   * this function in case there is no way to use the alias
   */
  std::string RepoManager::Impl::generateFilename( const RepoInfo & info ) const
  {
    std::string filename = info.alias();
    // replace slashes with underscores
    str::replaceAll( filename, "/", "_" );

    filename = Pathname(filename).extend(".repo").asString();
    MIL << "generating filename for repo [" << info.alias() << "] : '" << filename << "'" << endl;
    return filename;
  }

  std::string RepoManager::Impl::generateFilename( const ServiceInfo & info ) const
  {
    std::string filename = info.alias();
    // replace slashes with underscores
    str::replaceAll( filename, "/", "_" );

    filename = Pathname(filename).extend(".service").asString();
    MIL << "generating filename for service [" << info.alias() << "] : '" << filename << "'" << endl;
    return filename;
  }


  void RepoManager::Impl::init_knownServices()
  {
    Pathname dir = options.knownServicesPath;
    std::list<Pathname> entries;
    if (PathInfo(dir).isExist())
    {
      if ( filesystem::readdir( entries, dir, false ) != 0 )
      {
        // TranslatorExplanation '%s' is a pathname
        ZYPP_THROW(Exception(str::form(_("Failed to read directory '%s'"), dir.c_str())));
      }

      //str::regex allowedServiceExt("^\\.service(_[0-9]+)?$");
      for_(it, entries.begin(), entries.end() )
      {
        parser::ServiceFileReader(*it, ServiceCollector(services));
      }
    }

    repo::PluginServices(options.pluginsPath/"services", ServiceCollector(services));
  }

  void RepoManager::Impl::init_knownRepositories()
  {
    MIL << "start construct known repos" << endl;

    if ( PathInfo(options.knownReposPath).isExist() )
    {
      RepoInfoList repol = repositories_in_dir(options.knownReposPath);
      std::list<string> repo_esc_aliases;
      std::list<string> entries;
      for ( RepoInfoList::iterator it = repol.begin();
            it != repol.end();
            ++it )
      {
        // set the metadata path for the repo
        Pathname metadata_path = rawcache_path_for_repoinfo(options, (*it));
        (*it).setMetadataPath(metadata_path);

	// set the downloaded packages path for the repo
	Pathname packages_path = packagescache_path_for_repoinfo(options, (*it));
	(*it).setPackagesPath(packages_path);

        repos.insert(*it);
        repo_esc_aliases.push_back(it->escaped_alias());
      }

      // delete metadata folders without corresponding repo (e.g. old tmp directories)
      if ( filesystem::readdir( entries, options.repoRawCachePath, false ) == 0 )
      {
        std::set<string> oldfiles;
        repo_esc_aliases.sort();
        entries.sort();
        set_difference(entries.begin(), entries.end(), repo_esc_aliases.begin(), repo_esc_aliases.end(), std::inserter(oldfiles, oldfiles.end()));
        for_(it, oldfiles.begin(), oldfiles.end())
        {
          filesystem::recursive_rmdir(options.repoRawCachePath / *it);
        }
      }
    }

    MIL << "end construct known repos" << endl;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoManager
  //
  ///////////////////////////////////////////////////////////////////

  RepoManager::RepoManager( const RepoManagerOptions &opt )
  : _pimpl( new Impl(opt) )
  {}

  ////////////////////////////////////////////////////////////////////////////

  RepoManager::~RepoManager()
  {}

  ////////////////////////////////////////////////////////////////////////////

  bool RepoManager::repoEmpty() const
  { return _pimpl->repos.empty(); }

  RepoManager::RepoSizeType RepoManager::repoSize() const
  { return _pimpl->repos.size(); }

  RepoManager::RepoConstIterator RepoManager::repoBegin() const
  { return _pimpl->repos.begin(); }

  RepoManager::RepoConstIterator RepoManager::repoEnd() const
  { return _pimpl->repos.end(); }

  RepoInfo RepoManager::getRepo( const std::string & alias ) const
  {
    for_( it, repoBegin(), repoEnd() )
      if ( it->alias() == alias )
        return *it;
    return RepoInfo::noRepo;
  }

  bool RepoManager::hasRepo( const std::string & alias ) const
  {
    for_( it, repoBegin(), repoEnd() )
      if ( it->alias() == alias )
        return true;
    return false;
  }

  std::string RepoManager::makeStupidAlias( const Url & url_r )
  {
    std::string ret( url_r.getScheme() );
    if ( ret.empty() )
      ret = "repo-";
    else
      ret += "-";

    std::string host( url_r.getHost() );
    if ( ! host.empty() )
    {
      ret += host;
      ret += "-";
    }

    static Date::ValueType serial = Date::now();
    ret += Digest::digest( Digest::sha1(), str::hexstring( ++serial ) +url_r.asCompleteString() ).substr(0,8);
    return ret;
  }

  ////////////////////////////////////////////////////////////////////////////

  Pathname RepoManager::metadataPath( const RepoInfo &info ) const
  {
    return rawcache_path_for_repoinfo(_pimpl->options, info );
  }

  Pathname RepoManager::packagesPath( const RepoInfo &info ) const
  {
    return packagescache_path_for_repoinfo(_pimpl->options, info );
  }

  ////////////////////////////////////////////////////////////////////////////

  RepoStatus RepoManager::metadataStatus( const RepoInfo &info ) const
  {
    Pathname mediarootpath = rawcache_path_for_repoinfo( _pimpl->options, info );
    Pathname productdatapath = rawproductdata_path_for_repoinfo( _pimpl->options, info );
    RepoType repokind = info.type();
    RepoStatus status;

    switch ( repokind.toEnum() )
    {
      case RepoType::NONE_e:
      // unknown, probe the local metadata
        repokind = probe( productdatapath.asUrl() );
      break;
      default:
      break;
    }

    switch ( repokind.toEnum() )
    {
      case RepoType::RPMMD_e :
      {
        status = RepoStatus( productdatapath + "/repodata/repomd.xml");
      }
      break;

      case RepoType::YAST2_e :
      {
        status = RepoStatus( productdatapath + "/content") && (RepoStatus( mediarootpath + "/media.1/media"));
      }
      break;

      case RepoType::RPMPLAINDIR_e :
      {
        if ( PathInfo(Pathname(productdatapath + "/cookie")).isExist() )
          status = RepoStatus( productdatapath + "/cookie");
      }
      break;

      case RepoType::NONE_e :
	// Return default RepoStatus in case of RepoType::NONE
	// indicating it should be created?
        // ZYPP_THROW(RepoUnknownTypeException());
	break;
    }
    return status;
  }

  void RepoManager::touchIndexFile(const RepoInfo & info)
  {
    Pathname productdatapath = rawproductdata_path_for_repoinfo( _pimpl->options, info );

    RepoType repokind = info.type();
    if ( repokind.toEnum() == RepoType::NONE_e )
      // unknown, probe the local metadata
      repokind = probe( productdatapath.asUrl() );
    // if still unknown, just return
    if (repokind == RepoType::NONE_e)
      return;

    Pathname p;
    switch ( repokind.toEnum() )
    {
      case RepoType::RPMMD_e :
        p = Pathname(productdatapath + "/repodata/repomd.xml");
        break;

      case RepoType::YAST2_e :
        p = Pathname(productdatapath + "/content");
        break;

      case RepoType::RPMPLAINDIR_e :
        p = Pathname(productdatapath + "/cookie");
        break;

      case RepoType::NONE_e :
      default:
        break;
    }

    // touch the file, ignore error (they are logged anyway)
    filesystem::touch(p);
  }

  RepoManager::RefreshCheckStatus RepoManager::checkIfToRefreshMetadata(
                                              const RepoInfo &info,
                                              const Url &url,
                                              RawMetadataRefreshPolicy policy )
  {
    assert_alias(info);

    RepoStatus oldstatus;
    RepoStatus newstatus;

    try
    {
      MIL << "Going to try to check whether refresh is needed for " << url << endl;

      // first check old (cached) metadata
      Pathname mediarootpath = rawcache_path_for_repoinfo( _pimpl->options, info );
      filesystem::assert_dir(mediarootpath);
      oldstatus = metadataStatus(info);

      if ( oldstatus.empty() )
      {
        MIL << "No cached metadata, going to refresh" << endl;
        return REFRESH_NEEDED;
      }

      {
        std::string scheme( url.getScheme() );
        if ( scheme == "cd" || scheme == "dvd" )
        {
          MIL << "never refresh CD/DVD" << endl;
          return REPO_UP_TO_DATE;
        }
      }

      // now we've got the old (cached) status, we can decide repo.refresh.delay
      if (policy != RefreshForced && policy != RefreshIfNeededIgnoreDelay)
      {
        // difference in seconds
        double diff = difftime(
          (Date::ValueType)Date::now(),
          (Date::ValueType)oldstatus.timestamp()) / 60;

        DBG << "oldstatus: " << (Date::ValueType)oldstatus.timestamp() << endl;
        DBG << "current time: " << (Date::ValueType)Date::now() << endl;
        DBG << "last refresh = " << diff << " minutes ago" << endl;

        if ( diff < ZConfig::instance().repo_refresh_delay() )
        {
	  if ( diff < 0 )
	  {
	    WAR << "Repository '" << info.alias() << "' was refreshed in the future!" << endl;
	  }
	  else
	  {
	    MIL << "Repository '" << info.alias()
		<< "' has been refreshed less than repo.refresh.delay ("
		<< ZConfig::instance().repo_refresh_delay()
		<< ") minutes ago. Advising to skip refresh" << endl;
	    return REPO_CHECK_DELAYED;
	  }
        }
      }

      // To test the new matadta create temp dir as sibling of mediarootpath
      filesystem::TmpDir tmpdir( filesystem::TmpDir::makeSibling( mediarootpath ) );

      repo::RepoType repokind = info.type();
      // if the type is unknown, try probing.
      switch ( repokind.toEnum() )
      {
        case RepoType::NONE_e:
          // unknown, probe it \todo respect productdir
          repokind = probe( url, info.path() );
        break;
        default:
        break;
      }

      if ( ( repokind.toEnum() == RepoType::RPMMD_e ) ||
           ( repokind.toEnum() == RepoType::YAST2_e ) )
      {
        MediaSetAccess media(url);
        shared_ptr<repo::Downloader> downloader_ptr;

        if ( repokind.toEnum() == RepoType::RPMMD_e )
          downloader_ptr.reset(new yum::Downloader(info, mediarootpath));
        else
          downloader_ptr.reset( new susetags::Downloader(info, mediarootpath));

        RepoStatus newstatus = downloader_ptr->status(media);
        bool refresh = false;
        if ( oldstatus.checksum() == newstatus.checksum() )
        {
          MIL << "repo has not changed" << endl;
          if ( policy == RefreshForced )
          {
            MIL << "refresh set to forced" << endl;
            refresh = true;
          }
        }
        else
        {
          MIL << "repo has changed, going to refresh" << endl;
          refresh = true;
        }

        if (!refresh)
          touchIndexFile(info);

        return refresh ? REFRESH_NEEDED : REPO_UP_TO_DATE;
      }
      else if ( repokind.toEnum() == RepoType::RPMPLAINDIR_e )
      {
        MediaMounter media( url );
        RepoStatus newstatus = parser::plaindir::dirStatus( media.getPathName( info.path() ) );
        bool refresh = false;
        if ( oldstatus.checksum() == newstatus.checksum() )
        {
          MIL << "repo has not changed" << endl;
          if ( policy == RefreshForced )
          {
            MIL << "refresh set to forced" << endl;
            refresh = true;
          }
        }
        else
        {
          MIL << "repo has changed, going to refresh" << endl;
          refresh = true;
        }

        if (!refresh)
          touchIndexFile(info);

        return refresh ? REFRESH_NEEDED : REPO_UP_TO_DATE;
      }
      else
      {
        ZYPP_THROW(RepoUnknownTypeException(info));
      }
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      ERR << "refresh check failed for " << url << endl;
      ZYPP_RETHROW(e);
    }

    return REFRESH_NEEDED; // default
  }

  void RepoManager::refreshMetadata( const RepoInfo &info,
                                     RawMetadataRefreshPolicy policy,
                                     const ProgressData::ReceiverFnc & progress )
  {
    assert_alias(info);
    assert_urls(info);

    // we will throw this later if no URL checks out fine
    RepoException rexception(_PL("Valid metadata not found at specified URL",
                                 "Valid metadata not found at specified URLs",
				 info.baseUrlsSize() ) );

    // try urls one by one
    for ( RepoInfo::urls_const_iterator it = info.baseUrlsBegin(); it != info.baseUrlsEnd(); ++it )
    {
      try
      {
        Url url(*it);

        // check whether to refresh metadata
        // if the check fails for this url, it throws, so another url will be checked
        if (checkIfToRefreshMetadata(info, url, policy)!=REFRESH_NEEDED)
          return;

        MIL << "Going to refresh metadata from " << url << endl;

        repo::RepoType repokind = info.type();

        // if the type is unknown, try probing.
        switch ( repokind.toEnum() )
        {
          case RepoType::NONE_e:
            // unknown, probe it
            repokind = probe( *it, info.path() );

            if (repokind.toEnum() != RepoType::NONE_e)
            {
              // Adjust the probed type in RepoInfo
              info.setProbedType( repokind ); // lazy init!
              //save probed type only for repos in system
              for_( it, repoBegin(), repoEnd() )
              {
                if ( info.alias() == (*it).alias() )
                {
                  RepoInfo modifiedrepo = info;
                  modifiedrepo.setType( repokind );
                  modifyRepository( info.alias(), modifiedrepo );
                  break;
                }
              }
            }
          break;
          default:
          break;
        }

        Pathname mediarootpath = rawcache_path_for_repoinfo( _pimpl->options, info );
        if( filesystem::assert_dir(mediarootpath) )
        {
          Exception ex(str::form( _("Can't create %s"), mediarootpath.c_str()) );
          ZYPP_THROW(ex);
        }

        // create temp dir as sibling of mediarootpath
        filesystem::TmpDir tmpdir( filesystem::TmpDir::makeSibling( mediarootpath ) );
        if( tmpdir.path().empty() )
        {
          Exception ex(_("Can't create metadata cache directory."));
          ZYPP_THROW(ex);
        }

        if ( ( repokind.toEnum() == RepoType::RPMMD_e ) ||
             ( repokind.toEnum() == RepoType::YAST2_e ) )
        {
          MediaSetAccess media(url);
          shared_ptr<repo::Downloader> downloader_ptr;

          MIL << "Creating downloader for [ " << info.alias() << " ]" << endl;

          if ( repokind.toEnum() == RepoType::RPMMD_e )
            downloader_ptr.reset(new yum::Downloader(info, mediarootpath));
          else
            downloader_ptr.reset( new susetags::Downloader(info, mediarootpath) );

          /**
           * Given a downloader, sets the other repos raw metadata
           * path as cache paths for the fetcher, so if another
           * repo has the same file, it will not download it
           * but copy it from the other repository
           */
          for_( it, repoBegin(), repoEnd() )
          {
            Pathname cachepath(rawcache_path_for_repoinfo( _pimpl->options, *it ));
            if ( PathInfo(cachepath).isExist() )
              downloader_ptr->addCachePath(cachepath);
          }

          downloader_ptr->download( media, tmpdir.path() );
        }
        else if ( repokind.toEnum() == RepoType::RPMPLAINDIR_e )
        {
          MediaMounter media( url );
          RepoStatus newstatus = parser::plaindir::dirStatus( media.getPathName( info.path() ) );

          Pathname productpath( tmpdir.path() / info.path() );
          filesystem::assert_dir( productpath );
          std::ofstream file( (productpath/"cookie").c_str() );
          if ( !file )
          {
            // TranslatorExplanation '%s' is a filename
            ZYPP_THROW( Exception(str::form( _("Can't open file '%s' for writing."), (productpath/"cookie").c_str() )));
          }
          file << url;
          if ( ! info.path().empty() && info.path() != "/" )
            file << " (" << info.path() << ")";
          file << endl;
          file << newstatus.checksum() << endl;

          file.close();
        }
        else
        {
          ZYPP_THROW(RepoUnknownTypeException());
        }

        // ok we have the metadata, now exchange
        // the contents
	filesystem::exchange( tmpdir.path(), mediarootpath );

        // we are done.
        return;
      }
      catch ( const Exception &e )
      {
        ZYPP_CAUGHT(e);
        ERR << "Trying another url..." << endl;

        // remember the exception caught for the *first URL*
        // if all other URLs fail, the rexception will be thrown with the
        // cause of the problem of the first URL remembered
        if (it == info.baseUrlsBegin())
          rexception.remember(e);
      }
    } // for every url
    ERR << "No more urls..." << endl;
    ZYPP_THROW(rexception);
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::cleanMetadata( const RepoInfo &info,
                                   const ProgressData::ReceiverFnc & progressfnc )
  {
    ProgressData progress(100);
    progress.sendTo(progressfnc);

    filesystem::recursive_rmdir(rawcache_path_for_repoinfo(_pimpl->options, info));
    progress.toMax();
  }

  void RepoManager::cleanPackages( const RepoInfo &info,
                                   const ProgressData::ReceiverFnc & progressfnc )
  {
    ProgressData progress(100);
    progress.sendTo(progressfnc);

    filesystem::recursive_rmdir(packagescache_path_for_repoinfo(_pimpl->options, info));
    progress.toMax();
  }

  void RepoManager::buildCache( const RepoInfo &info,
                                CacheBuildPolicy policy,
                                const ProgressData::ReceiverFnc & progressrcv )
  {
    assert_alias(info);
    Pathname mediarootpath = rawcache_path_for_repoinfo( _pimpl->options, info );
    Pathname productdatapath = rawproductdata_path_for_repoinfo( _pimpl->options, info );

    if( filesystem::assert_dir(_pimpl->options.repoCachePath) )
    {
      Exception ex(str::form( _("Can't create %s"), _pimpl->options.repoCachePath.c_str()) );
      ZYPP_THROW(ex);
    }
    RepoStatus raw_metadata_status = metadataStatus(info);
    if ( raw_metadata_status.empty() )
    {
       /* if there is no cache at this point, we refresh the raw
          in case this is the first time - if it's !autorefresh,
          we may still refresh */
      refreshMetadata(info, RefreshIfNeeded, progressrcv );
      raw_metadata_status = metadataStatus(info);
    }

    bool needs_cleaning = false;
    if ( isCached( info ) )
    {
      MIL << info.alias() << " is already cached." << endl;
      RepoStatus cache_status = cacheStatus(info);

      if ( cache_status.checksum() == raw_metadata_status.checksum() )
      {
        MIL << info.alias() << " cache is up to date with metadata." << endl;
        if ( policy == BuildIfNeeded ) {
          return;
        }
        else {
          MIL << info.alias() << " cache rebuild is forced" << endl;
        }
      }

      needs_cleaning = true;
    }

    ProgressData progress(100);
    callback::SendReport<ProgressReport> report;
    progress.sendTo( ProgressReportAdaptor( progressrcv, report ) );
    progress.name(str::form(_("Building repository '%s' cache"), info.label().c_str()));
    progress.toMin();

    if (needs_cleaning)
    {
      cleanCache(info);
    }

    MIL << info.alias() << " building cache..." << info.type() << endl;

    Pathname base = solv_path_for_repoinfo( _pimpl->options, info);

    if( filesystem::assert_dir(base) )
    {
      Exception ex(str::form( _("Can't create %s"), base.c_str()) );
      ZYPP_THROW(ex);
    }

    if( ! PathInfo(base).userMayW() )
    {
      Exception ex(str::form( _("Can't create cache at %s - no writing permissions."), base.c_str()) );
      ZYPP_THROW(ex);
    }
    Pathname solvfile = base / "solv";

    // do we have type?
    repo::RepoType repokind = info.type();

    // if the type is unknown, try probing.
    switch ( repokind.toEnum() )
    {
      case RepoType::NONE_e:
        // unknown, probe the local metadata
        repokind = probe( productdatapath.asUrl() );
      break;
      default:
      break;
    }

    MIL << "repo type is " << repokind << endl;

    switch ( repokind.toEnum() )
    {
      case RepoType::RPMMD_e :
      case RepoType::YAST2_e :
      case RepoType::RPMPLAINDIR_e :
      {
        // Take care we unlink the solvfile on exception
        ManagedFile guard( solvfile, filesystem::unlink );
        scoped_ptr<MediaMounter> forPlainDirs;

        ExternalProgram::Arguments cmd;
        cmd.push_back( "repo2solv.sh" );

        // repo2solv expects -o as 1st arg!
        cmd.push_back( "-o" );
        cmd.push_back( solvfile.asString() );

        if ( repokind == RepoType::RPMPLAINDIR )
        {
          forPlainDirs.reset( new MediaMounter( *info.baseUrlsBegin() ) );
          // recusive for plaindir as 2nd arg!
          cmd.push_back( "-R" );
          // FIXME this does only work form dir: URLs
          cmd.push_back( forPlainDirs->getPathName( info.path() ).c_str() );
        }
        else
          cmd.push_back( productdatapath.asString() );

        ExternalProgram prog( cmd, ExternalProgram::Stderr_To_Stdout );
        std::string errdetail;

        for ( std::string output( prog.receiveLine() ); output.length(); output = prog.receiveLine() ) {
          WAR << "  " << output;
          if ( errdetail.empty() ) {
            errdetail = prog.command();
            errdetail += '\n';
          }
          errdetail += output;
        }

        int ret = prog.close();
        if ( ret != 0 )
        {
          RepoException ex(str::form( _("Failed to cache repo (%d)."), ret ));
          ex.remember( errdetail );
          ZYPP_THROW(ex);
        }

        // We keep it.
        guard.resetDispose();
      }
      break;
      default:
        ZYPP_THROW(RepoUnknownTypeException( _("Unhandled repository type") ));
      break;
    }
    // update timestamp and checksum
    setCacheStatus(info, raw_metadata_status);
    MIL << "Commit cache.." << endl;
    progress.toMax();
  }

  ////////////////////////////////////////////////////////////////////////////

  repo::RepoType RepoManager::probe( const Url & url ) const
  { return probe( url, Pathname() ); }

  repo::RepoType RepoManager::probe( const Url & url, const Pathname & path  ) const
  {
    MIL << "going to probe the repo type at " << url << " (" << path << ")" << endl;

    if ( url.getScheme() == "dir" && ! PathInfo( url.getPathName()/path ).isDir() )
    {
      // Handle non existing local directory in advance, as
      // MediaSetAccess does not support it.
      MIL << "Probed type NONE (not exists) at " << url << " (" << path << ")" << endl;
      return repo::RepoType::NONE;
    }

    // prepare exception to be thrown if the type could not be determined
    // due to a media exception. We can't throw right away, because of some
    // problems with proxy servers returning an incorrect error
    // on ftp file-not-found(bnc #335906). Instead we'll check another types
    // before throwing.

    // TranslatorExplanation '%s' is an URL
    RepoException enew(str::form( _("Error trying to read from '%s'"), url.asString().c_str() ));
    bool gotMediaException = false;
    try
    {
      MediaSetAccess access(url);
      try
      {
        if ( access.doesFileExist(path/"/repodata/repomd.xml") )
        {
          MIL << "Probed type RPMMD at " << url << " (" << path << ")" << endl;
          return repo::RepoType::RPMMD;
        }
      }
      catch ( const media::MediaException &e )
      {
        ZYPP_CAUGHT(e);
        DBG << "problem checking for repodata/repomd.xml file" << endl;
        enew.remember(e);
        gotMediaException = true;
      }

      try
      {
        if ( access.doesFileExist(path/"/content") )
        {
          MIL << "Probed type YAST2 at " << url << " (" << path << ")" << endl;
          return repo::RepoType::YAST2;
        }
      }
      catch ( const media::MediaException &e )
      {
        ZYPP_CAUGHT(e);
        DBG << "problem checking for content file" << endl;
        enew.remember(e);
        gotMediaException = true;
      }

      // if it is a non-downloading URL denoting a directory
      if ( ! url.schemeIsDownloading() )
      {
        MediaMounter media( url );
        if ( PathInfo(media.getPathName()/path).isDir() )
        {
          // allow empty dirs for now
          MIL << "Probed type RPMPLAINDIR at " << url << " (" << path << ")" << endl;
          return repo::RepoType::RPMPLAINDIR;
        }
      }
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      // TranslatorExplanation '%s' is an URL
      Exception enew(str::form( _("Unknown error reading from '%s'"), url.asString().c_str() ));
      enew.remember(e);
      ZYPP_THROW(enew);
    }

    if (gotMediaException)
      ZYPP_THROW(enew);

    MIL << "Probed type NONE at " << url << " (" << path << ")" << endl;
    return repo::RepoType::NONE;
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::cleanCacheDirGarbage( const ProgressData::ReceiverFnc & progressrcv )
  {
    MIL << "Going to clean up garbage in cache dirs" << endl;

    ProgressData progress(300);
    progress.sendTo(progressrcv);
    progress.toMin();

    std::list<Pathname> cachedirs;
    cachedirs.push_back(_pimpl->options.repoRawCachePath);
    cachedirs.push_back(_pimpl->options.repoPackagesCachePath);
    cachedirs.push_back(_pimpl->options.repoSolvCachePath);

    for_( dir, cachedirs.begin(), cachedirs.end() )
    {
      if ( PathInfo(*dir).isExist() )
      {
        std::list<Pathname> entries;
        if ( filesystem::readdir( entries, *dir, false ) != 0 )
          // TranslatorExplanation '%s' is a pathname
          ZYPP_THROW(Exception(str::form(_("Failed to read directory '%s'"), dir->c_str())));

        unsigned sdircount   = entries.size();
        unsigned sdircurrent = 1;
        for_( subdir, entries.begin(), entries.end() )
        {
          // if it does not belong known repo, make it disappear
          bool found = false;
          for_( r, repoBegin(), repoEnd() )
            if ( subdir->basename() == r->escaped_alias() )
            { found = true; break; }

          if ( ! found )
            filesystem::recursive_rmdir( *subdir );

          progress.set( progress.val() + sdircurrent * 100 / sdircount );
          ++sdircurrent;
        }
      }
      else
        progress.set( progress.val() + 100 );
    }
    progress.toMax();
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::cleanCache( const RepoInfo &info,
                                const ProgressData::ReceiverFnc & progressrcv )
  {
    ProgressData progress(100);
    progress.sendTo(progressrcv);
    progress.toMin();

    MIL << "Removing raw metadata cache for " << info.alias() << endl;
    filesystem::recursive_rmdir(solv_path_for_repoinfo(_pimpl->options, info));

    progress.toMax();
  }

  ////////////////////////////////////////////////////////////////////////////

  bool RepoManager::isCached( const RepoInfo &info ) const
  {
    return PathInfo(solv_path_for_repoinfo( _pimpl->options, info ) / "solv").isExist();
  }

  RepoStatus RepoManager::cacheStatus( const RepoInfo &info ) const
  {

    Pathname cookiefile = solv_path_for_repoinfo(_pimpl->options, info) / "cookie";

    return RepoStatus::fromCookieFile(cookiefile);
  }

  void RepoManager::setCacheStatus( const RepoInfo &info, const RepoStatus &status )
  {
    Pathname base = solv_path_for_repoinfo(_pimpl->options, info);
    filesystem::assert_dir(base);
    Pathname cookiefile = base / "cookie";

    status.saveToCookieFile(cookiefile);
  }

  void RepoManager::loadFromCache( const RepoInfo & info,
                                   const ProgressData::ReceiverFnc & progressrcv )
  {
    assert_alias(info);
    Pathname solvfile = solv_path_for_repoinfo(_pimpl->options, info) / "solv";

    if ( ! PathInfo(solvfile).isExist() )
      ZYPP_THROW(RepoNotCachedException(info));

    sat::Pool::instance().reposErase( info.alias() );
    try
    {
      Repository repo = sat::Pool::instance().addRepoSolv( solvfile, info );
      // test toolversion in order to rebuild solv file in case
      // it was written by an old libsolv-tool parser.
      //
      // Known version strings used:
      //  - <no string>
      //  - "1.0"
      //
      sat::LookupRepoAttr toolversion( sat::SolvAttr::repositoryToolVersion, repo );
      if ( toolversion.begin().asString().empty() )
      {
        repo.eraseFromPool();
        ZYPP_THROW(Exception("Solv-file was created by old parser."));
      }
      // else: up-to-date (or even newer).
    }
    catch ( const Exception & exp )
    {
      ZYPP_CAUGHT( exp );
      MIL << "Try to handle exception by rebuilding the solv-file" << endl;
      cleanCache( info, progressrcv );
      buildCache( info, BuildIfNeeded, progressrcv );

      sat::Pool::instance().addRepoSolv( solvfile, info );
    }
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::addRepository( const RepoInfo &info,
                                   const ProgressData::ReceiverFnc & progressrcv )
  {
    assert_alias(info);

    ProgressData progress(100);
    callback::SendReport<ProgressReport> report;
    progress.sendTo( ProgressReportAdaptor( progressrcv, report ) );
    progress.name(str::form(_("Adding repository '%s'"), info.label().c_str()));
    progress.toMin();

    MIL << "Try adding repo " << info << endl;

    RepoInfo tosave = info;
    if(_pimpl->repos.find(tosave)!= _pimpl->repos.end())
        ZYPP_THROW(RepoAlreadyExistsException(info));

    // check the first url for now
    if ( _pimpl->options.probe )
    {
      DBG << "unknown repository type, probing" << endl;

      RepoType probedtype;
      probedtype = probe( *tosave.baseUrlsBegin(), info.path() );
      if ( tosave.baseUrlsSize() > 0 )
      {
        if ( probedtype == RepoType::NONE )
          ZYPP_THROW(RepoUnknownTypeException());
        else
          tosave.setType(probedtype);
      }
    }

    progress.set(50);

    // assert the directory exists
    filesystem::assert_dir(_pimpl->options.knownReposPath);

    Pathname repofile = _pimpl->generateNonExistingName(
        _pimpl->options.knownReposPath, _pimpl->generateFilename(tosave));
    // now we have a filename that does not exists
    MIL << "Saving repo in " << repofile << endl;

    std::ofstream file(repofile.c_str());
    if (!file)
    {
      // TranslatorExplanation '%s' is a filename
      ZYPP_THROW( Exception(str::form( _("Can't open file '%s' for writing."), repofile.c_str() )));
    }

    tosave.dumpAsIniOn(file);
    tosave.setFilepath(repofile);
    tosave.setMetadataPath( metadataPath( tosave ) );
    tosave.setPackagesPath( packagesPath( tosave ) );
    {
      // We chould fix the API as we must injet those paths
      // into the repoinfo in order to keep it usable.
      RepoInfo & oinfo( const_cast<RepoInfo &>(info) );
      oinfo.setMetadataPath( metadataPath( tosave ) );
      oinfo.setPackagesPath( packagesPath( tosave ) );
    }
    _pimpl->repos.insert(tosave);

    progress.set(90);

    // check for credentials in Urls
    bool havePasswords = false;
    for_( urlit, tosave.baseUrlsBegin(), tosave.baseUrlsEnd() )
      if ( urlit->hasCredentialsInAuthority() )
      {
        havePasswords = true;
        break;
      }
    // save the credentials
    if ( havePasswords )
    {
      media::CredentialManager cm(
          media::CredManagerOptions(_pimpl->options.rootDir) );

      for_(urlit, tosave.baseUrlsBegin(), tosave.baseUrlsEnd())
        if (urlit->hasCredentialsInAuthority())
          //! \todo use a method calling UI callbacks to ask where to save creds?
          cm.saveInUser(media::AuthData(*urlit));
    }

    HistoryLog().addRepository(tosave);

    progress.toMax();
    MIL << "done" << endl;
  }

  void RepoManager::addRepositories( const Url &url,
                                     const ProgressData::ReceiverFnc & progressrcv )
  {
    std::list<RepoInfo> repos = readRepoFile(url);
    for ( std::list<RepoInfo>::const_iterator it = repos.begin();
          it != repos.end();
          ++it )
    {
      // look if the alias is in the known repos.
      for_ ( kit, repoBegin(), repoEnd() )
      {
        if ( (*it).alias() == (*kit).alias() )
        {
          ERR << "To be added repo " << (*it).alias() << " conflicts with existing repo " << (*kit).alias() << endl;
          ZYPP_THROW(RepoAlreadyExistsException(*it));
        }
      }
    }

    std::string filename = Pathname(url.getPathName()).basename();

    if ( filename == Pathname() )
    {
      // TranslatorExplanation '%s' is an URL
      ZYPP_THROW(RepoException(str::form( _("Invalid repo file name at '%s'"), url.asString().c_str() )));
    }

    // assert the directory exists
    filesystem::assert_dir(_pimpl->options.knownReposPath);

    Pathname repofile = _pimpl->generateNonExistingName(_pimpl->options.knownReposPath, filename);
    // now we have a filename that does not exists
    MIL << "Saving " << repos.size() << " repo" << ( repos.size() ? "s" : "" ) << " in " << repofile << endl;

    std::ofstream file(repofile.c_str());
    if (!file)
    {
      // TranslatorExplanation '%s' is a filename
      ZYPP_THROW( Exception(str::form( _("Can't open file '%s' for writing."), repofile.c_str() )));
    }

    for ( std::list<RepoInfo>::iterator it = repos.begin();
          it != repos.end();
          ++it )
    {
      MIL << "Saving " << (*it).alias() << endl;
      it->setFilepath(repofile.asString());
      it->dumpAsIniOn(file);
      _pimpl->repos.insert(*it);

      HistoryLog(_pimpl->options.rootDir).addRepository(*it);
    }

    MIL << "done" << endl;
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::removeRepository( const RepoInfo & info,
                                      const ProgressData::ReceiverFnc & progressrcv)
  {
    ProgressData progress;
    callback::SendReport<ProgressReport> report;
    progress.sendTo( ProgressReportAdaptor( progressrcv, report ) );
    progress.name(str::form(_("Removing repository '%s'"), info.label().c_str()));

    MIL << "Going to delete repo " << info.alias() << endl;

    for_( it, repoBegin(), repoEnd() )
    {
      // they can be the same only if the provided is empty, that means
      // the provided repo has no alias
      // then skip
      if ( (!info.alias().empty()) && ( info.alias() != (*it).alias() ) )
        continue;

      // TODO match by url

      // we have a matcing repository, now we need to know
      // where it does come from.
      RepoInfo todelete = *it;
      if (todelete.filepath().empty())
      {
        ZYPP_THROW(RepoException( _("Can't figure out where the repo is stored.") ));
      }
      else
      {
        // figure how many repos are there in the file:
        std::list<RepoInfo> filerepos = repositories_in_file(todelete.filepath());
        if ( (filerepos.size() == 1) && ( filerepos.front().alias() == todelete.alias() ) )
        {
          // easy, only this one, just delete the file
          if ( filesystem::unlink(todelete.filepath()) != 0 )
          {
            // TranslatorExplanation '%s' is a filename
            ZYPP_THROW(RepoException(str::form( _("Can't delete '%s'"), todelete.filepath().c_str() )));
          }
          MIL << todelete.alias() << " sucessfully deleted." << endl;
        }
        else
        {
          // there are more repos in the same file
          // write them back except the deleted one.
          //TmpFile tmp;
          //std::ofstream file(tmp.path().c_str());

          // assert the directory exists
          filesystem::assert_dir(todelete.filepath().dirname());

          std::ofstream file(todelete.filepath().c_str());
          if (!file)
          {
            // TranslatorExplanation '%s' is a filename
            ZYPP_THROW( Exception(str::form( _("Can't open file '%s' for writing."), todelete.filepath().c_str() )));
          }
          for ( std::list<RepoInfo>::const_iterator fit = filerepos.begin();
                fit != filerepos.end();
                ++fit )
          {
            if ( (*fit).alias() != todelete.alias() )
              (*fit).dumpAsIniOn(file);
          }
        }

        CombinedProgressData subprogrcv(progress, 70);
        CombinedProgressData cleansubprogrcv(progress, 30);
        // now delete it from cache
        if ( isCached(todelete) )
          cleanCache( todelete, subprogrcv);
        // now delete metadata (#301037)
        cleanMetadata( todelete, cleansubprogrcv);
        _pimpl->repos.erase(todelete);
        MIL << todelete.alias() << " sucessfully deleted." << endl;
        HistoryLog(_pimpl->options.rootDir).removeRepository(todelete);
        return;
      } // else filepath is empty

    }
    // should not be reached on a sucess workflow
    ZYPP_THROW(RepoNotFoundException(info));
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::modifyRepository( const std::string &alias,
                                      const RepoInfo & newinfo_r,
                                      const ProgressData::ReceiverFnc & progressrcv )
  {
    RepoInfo toedit = getRepositoryInfo(alias);
    RepoInfo newinfo( newinfo_r ); // need writable copy to upadte housekeeping data

    // check if the new alias already exists when renaming the repo
    if ( alias != newinfo.alias() && hasRepo( newinfo.alias() ) )
    {
      ZYPP_THROW(RepoAlreadyExistsException(newinfo));
    }

    if (toedit.filepath().empty())
    {
      ZYPP_THROW(RepoException( _("Can't figure out where the repo is stored.") ));
    }
    else
    {
      // figure how many repos are there in the file:
      std::list<RepoInfo> filerepos = repositories_in_file(toedit.filepath());

      // there are more repos in the same file
      // write them back except the deleted one.
      //TmpFile tmp;
      //std::ofstream file(tmp.path().c_str());

      // assert the directory exists
      filesystem::assert_dir(toedit.filepath().dirname());

      std::ofstream file(toedit.filepath().c_str());
      if (!file)
      {
        // TranslatorExplanation '%s' is a filename
        ZYPP_THROW( Exception(str::form( _("Can't open file '%s' for writing."), toedit.filepath().c_str() )));
      }
      for ( std::list<RepoInfo>::const_iterator fit = filerepos.begin();
            fit != filerepos.end();
            ++fit )
      {
          // if the alias is different, dump the original
          // if it is the same, dump the provided one
          if ( (*fit).alias() != toedit.alias() )
            (*fit).dumpAsIniOn(file);
          else
            newinfo.dumpAsIniOn(file);
      }

      newinfo.setFilepath(toedit.filepath());
      _pimpl->repos.erase(toedit);
      _pimpl->repos.insert(newinfo);
      HistoryLog(_pimpl->options.rootDir).modifyRepository(toedit, newinfo);
      MIL << "repo " << alias << " modified" << endl;
    }
  }

  ////////////////////////////////////////////////////////////////////////////

  RepoInfo RepoManager::getRepositoryInfo( const std::string &alias,
                                           const ProgressData::ReceiverFnc & progressrcv )
  {
    RepoInfo info;
    info.setAlias(alias);
    RepoConstIterator it = _pimpl->repos.find( info );
    if( it == repoEnd() )
      ZYPP_THROW(RepoNotFoundException(info));
    else
      return *it;
  }

  ////////////////////////////////////////////////////////////////////////////

  RepoInfo RepoManager::getRepositoryInfo( const Url & url,
                                           const url::ViewOption & urlview,
                                           const ProgressData::ReceiverFnc & progressrcv )
  {
    for_( it, repoBegin(), repoEnd() )
    {
      for(RepoInfo::urls_const_iterator urlit = (*it).baseUrlsBegin();
          urlit != (*it).baseUrlsEnd();
          ++urlit)
      {
        if ((*urlit).asString(urlview) == url.asString(urlview))
          return *it;
      }
    }
    RepoInfo info;
    info.setBaseUrl(url);
    ZYPP_THROW(RepoNotFoundException(info));
  }

  ////////////////////////////////////////////////////////////////////////////
  //
  // Services
  //
  ////////////////////////////////////////////////////////////////////////////

  bool RepoManager::serviceEmpty() const
  { return _pimpl->services.empty(); }

  RepoManager::ServiceSizeType RepoManager::serviceSize() const
  { return _pimpl->services.size(); }

  RepoManager::ServiceConstIterator RepoManager::serviceBegin() const
  { return _pimpl->services.begin(); }

  RepoManager::ServiceConstIterator RepoManager::serviceEnd() const
  { return _pimpl->services.end(); }

  ServiceInfo RepoManager::getService( const std::string & alias ) const
  {
    for_( it, serviceBegin(), serviceEnd() )
      if ( it->alias() == alias )
        return *it;
    return ServiceInfo::noService;
  }

  bool RepoManager::hasService( const std::string & alias ) const
  {
    for_( it, serviceBegin(), serviceEnd() )
      if ( it->alias() == alias )
        return true;
    return false;
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::addService( const std::string & alias, const Url & url )
  {
    addService( ServiceInfo(alias, url) );
  }

  void RepoManager::addService( const ServiceInfo & service )
  {
    assert_alias( service );

    // check if service already exists
    if ( hasService( service.alias() ) )
      ZYPP_THROW( ServiceAlreadyExistsException( service ) );

    // Writable ServiceInfo is needed to save the location
    // of the .service file. Finaly insert into the service list.
    ServiceInfo toSave( service );
    _pimpl->saveService( toSave );
    _pimpl->services.insert( toSave );

    // check for credentials in Url (username:password, not ?credentials param)
    if ( toSave.url().hasCredentialsInAuthority() )
    {
      media::CredentialManager cm(
          media::CredManagerOptions(_pimpl->options.rootDir) );

      //! \todo use a method calling UI callbacks to ask where to save creds?
      cm.saveInUser(media::AuthData(toSave.url()));
    }

    MIL << "added service " << toSave.alias() << endl;
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::removeService( const std::string & alias )
  {
    MIL << "Going to delete repo " << alias << endl;

    const ServiceInfo & service = getService( alias );

    Pathname location = service.filepath();
    if( location.empty() )
    {
      ZYPP_THROW(RepoException( _("Can't figure out where the service is stored.") ));
    }

    ServiceSet tmpSet;
    parser::ServiceFileReader( location, ServiceCollector(tmpSet) );

    // only one service definition in the file
    if ( tmpSet.size() == 1 )
    {
      if ( filesystem::unlink(location) != 0 )
      {
        // TranslatorExplanation '%s' is a filename
        ZYPP_THROW(RepoException(str::form( _("Can't delete '%s'"), location.c_str() )));
      }
      MIL << alias << " sucessfully deleted." << endl;
    }
    else
    {
      filesystem::assert_dir(location.dirname());

      std::ofstream file(location.c_str());
      if( !file )
      {
        // TranslatorExplanation '%s' is a filename
        ZYPP_THROW( Exception(str::form( _("Can't open file '%s' for writing."), location.c_str() )));
      }

      for_(it, tmpSet.begin(), tmpSet.end())
      {
        if( it->alias() != alias )
          it->dumpAsIniOn(file);
      }

      MIL << alias << " sucessfully deleted from file " << location <<  endl;
    }

    // now remove all repositories added by this service
    RepoCollector rcollector;
    getRepositoriesInService( alias,
      boost::make_function_output_iterator(
          bind( &RepoCollector::collect, &rcollector, _1 ) ) );
    // cannot do this directly in getRepositoriesInService - would invalidate iterators
    for_(rit, rcollector.repos.begin(), rcollector.repos.end())
      removeRepository(*rit);
  }

  void RepoManager::removeService( const ServiceInfo & service )
  { removeService(service.alias()); }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::refreshServices()
  {
    // copy the set of services since refreshService
    // can eventually invalidate the iterator
    ServiceSet services( serviceBegin(), serviceEnd() );
    for_( it, services.begin(), services.end() )
    {
      if ( !it->enabled() )
        continue;

      try {
	refreshService(*it);
      }
      catch ( const repo::ServicePluginInformalException & e )
      { ;/* ignore ServicePluginInformalException */ }
    }
  }

  void RepoManager::refreshService( const ServiceInfo & service )
  { refreshService( service.alias() ); }

  void RepoManager::refreshService( const std::string & alias )
  {
    ServiceInfo service( getService( alias ) );
    assert_alias( service );
    assert_url( service );
    // NOTE: It might be necessary to modify and rewrite the service info.
    // Either when probing the type, or when adjusting the repositories
    // enable/disable state.:
    bool serviceModified = false;
    MIL << "Going to refresh service '" << service.alias() << "', url: "<< service.url() << endl;

    //! \todo add callbacks for apps (start, end, repo removed, repo added, repo changed)

    // if the type is unknown, try probing.
    if ( service.type() == repo::ServiceType::NONE )
    {
      repo::ServiceType type = probeService( service.url() );
      if ( type != ServiceType::NONE )
      {
        service.setProbedType( type ); // lazy init!
        serviceModified = true;
      }
    }

    // get target distro identifier
    std::string servicesTargetDistro = _pimpl->options.servicesTargetDistro;
    if ( servicesTargetDistro.empty() )
    {
      servicesTargetDistro = Target::targetDistribution( Pathname() );
    }
    DBG << "ServicesTargetDistro: " << servicesTargetDistro << endl;

    // parse it
    RepoCollector collector(servicesTargetDistro);
    // FIXME Ugly hack: ServiceRepos may throw ServicePluginInformalException
    // which is actually a notification. Using an exception for this
    // instead of signal/callback is bad. Needs to be fixed here, in refreshServices()
    // and in zypper.
    std::pair<DefaultIntegral<bool,false>, repo::ServicePluginInformalException> uglyHack;
    try {
      ServiceRepos repos(service, bind( &RepoCollector::collect, &collector, _1 ));
    }
    catch ( const repo::ServicePluginInformalException & e )
    {
      /* ignore ServicePluginInformalException and throw later */
      uglyHack.first = true;
      uglyHack.second = e;
    }

    // set service alias and base url for all collected repositories
    for_( it, collector.repos.begin(), collector.repos.end() )
    {
      // if the repo url was not set by the repoindex parser, set service's url
      Url url;

      if ( it->baseUrlsEmpty() )
        url = service.url();
      else
      {
        // service repo can contain only one URL now, so no need to iterate.
        url = *it->baseUrlsBegin();
      }

      // libzypp currently has problem with separate url + path handling
      // so just append the path to the baseurl
      if ( !it->path().empty() )
      {
        Pathname path(url.getPathName());
        path /= it->path();
        url.setPathName( path.asString() );
        it->setPath("");
      }

      // Prepend service alias:
      it->setAlias( str::form( "%s:%s", service.alias().c_str(), it->alias().c_str() ) );

      // save the url
      it->setBaseUrl( url );
      // set refrence to the parent service
      it->setService( service.alias() );
    }

    ////////////////////////////////////////////////////////////////////////////
    // Now compare collected repos with the ones in the system...
    //
    RepoInfoList oldRepos;
    getRepositoriesInService( service.alias(), std::back_inserter( oldRepos ) );

    // find old repositories to remove...
    for_( it, oldRepos.begin(), oldRepos.end() )
    {
      if ( ! foundAliasIn( it->alias(), collector.repos ) )
      {
        if ( it->enabled() && ! service.repoToDisableFind( it->alias() ) )
        {
          DBG << "Service removes enabled repo " << it->alias() << endl;
          service.addRepoToEnable( it->alias() );
          serviceModified = true;
        }
        else
        {
          DBG << "Service removes disabled repo " << it->alias() << endl;
        }
        removeRepository( *it );
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    // create missing repositories and modify exising ones if needed...
    for_( it, collector.repos.begin(), collector.repos.end() )
    {
      // Service explicitly requests the repo being enabled?
      // Service explicitly requests the repo being disabled?
      // And hopefully not both ;) If so, enable wins.
      bool beEnabled = service.repoToEnableFind( it->alias() );
      bool beDisabled = service.repoToDisableFind( it->alias() );

      // Make sure the service repo is created with the
      // appropriate enable
      if ( beEnabled ) it->setEnabled(true);
      if ( beDisabled ) it->setEnabled(false);

      if ( beEnabled )
      {
        // Remove from enable request list.
        // NOTE: repoToDisable is handled differently.
        //       It gets cleared on each refresh.
        service.delRepoToEnable( it->alias() );
        serviceModified = true;
      }

      RepoInfoList::iterator oldRepo( findAlias( it->alias(), oldRepos ) );
      if ( oldRepo == oldRepos.end() )
      {
        // Not found in oldRepos ==> a new repo to add

        // At that point check whether a repo with the same alias
        // exists outside this service. Maybe forcefully re-alias
        // the existing repo?
        DBG << "Service adds repo " << it->alias() << " " << (it->enabled()?"enabled":"disabled") << endl;
        addRepository( *it );

        // save repo credentials
        // ma@: task for modifyRepository?
      }
      else
      {
        // ==> an exising repo to check
        bool oldRepoModified = false;

        // changed enable?
        if ( beEnabled )
        {
          if ( ! oldRepo->enabled() )
          {
            DBG << "Service repo " << it->alias() << " gets enabled" << endl;
            oldRepo->setEnabled( true );
            oldRepoModified = true;
          }
          else
          {
            DBG << "Service repo " << it->alias() << " stays enabled" << endl;
          }
        }
        else if ( beDisabled )
        {
          if ( oldRepo->enabled() )
          {
            DBG << "Service repo " << it->alias() << " gets disabled" << endl;
            oldRepo->setEnabled( false );
            oldRepoModified = true;
          }
          else
          {
            DBG << "Service repo " << it->alias() << " stays disabled" << endl;
          }
        }
        else
        {
          DBG << "Service repo " << it->alias() << " stays " <<  (oldRepo->enabled()?"enabled":"disabled") << endl;
        }

        // changed url?
        // service repo can contain only one URL now, so no need to iterate.
        if ( oldRepo->url() != it->url() )
        {
          DBG << "Service repo " << it->alias() << " gets new URL " << it->url() << endl;
          oldRepo->setBaseUrl( it->url() );
          oldRepoModified = true;
        }

        // save if modified:
        if ( oldRepoModified )
        {
          modifyRepository( oldRepo->alias(), *oldRepo );
        }
      }
    }

    // Unlike reposToEnable, reposToDisable is always cleared after refresh.
    if ( ! service.reposToDisableEmpty() )
    {
      service.clearReposToDisable();
      serviceModified = true;
    }

    ////////////////////////////////////////////////////////////////////////////
    // save service if modified:
    if ( serviceModified )
    {
      // write out modified service file.
      modifyService( service.alias(), service );
    }

    if ( uglyHack.first )
    {
      throw( uglyHack.second ); // intentionally not ZYPP_THROW
    }
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::modifyService(const std::string & oldAlias, const ServiceInfo & newService)
  {
    MIL << "Going to modify service " << oldAlias << endl;

    // we need a writable copy to link it to the file where
    // it is saved if we modify it
    ServiceInfo service(newService);

    if ( service.type() == ServiceType::PLUGIN )
    {
        MIL << "Not modifying plugin service '" << oldAlias << "'" << endl;
        return;
    }

    const ServiceInfo & oldService = getService(oldAlias);

    Pathname location = oldService.filepath();
    if( location.empty() )
    {
      ZYPP_THROW(RepoException( _("Can't figure out where the service is stored.") ));
    }

    // remember: there may multiple services being defined in one file:
    ServiceSet tmpSet;
    parser::ServiceFileReader( location, ServiceCollector(tmpSet) );

    filesystem::assert_dir(location.dirname());
    std::ofstream file(location.c_str());
    for_(it, tmpSet.begin(), tmpSet.end())
    {
      if( *it != oldAlias )
        it->dumpAsIniOn(file);
    }
    service.dumpAsIniOn(file);
    file.close();
    service.setFilepath(location);

    _pimpl->services.erase(oldAlias);
    _pimpl->services.insert(service);

    // changed properties affecting also repositories
    if( oldAlias != service.alias()                    // changed alias
        || oldService.enabled() != service.enabled()   // changed enabled status
      )
    {
      std::vector<RepoInfo> toModify;
      getRepositoriesInService(oldAlias, std::back_inserter(toModify));
      for_( it, toModify.begin(), toModify.end() )
      {
        if (oldService.enabled() && !service.enabled())
          it->setEnabled(false);
        else if (!oldService.enabled() && service.enabled())
        {
          //! \todo do nothing? the repos will be enabled on service refresh
          //! \todo how to know the service needs a (auto) refresh????
        }
        else
          it->setService(service.alias());
        modifyRepository(it->alias(), *it);
      }
    }

    //! \todo refresh the service automatically if url is changed?
  }

  ////////////////////////////////////////////////////////////////////////////

  repo::ServiceType RepoManager::probeService( const Url &url ) const
  {
    try
    {
      MediaSetAccess access(url);
      if ( access.doesFileExist("/repo/repoindex.xml") )
        return repo::ServiceType::RIS;
    }
    catch ( const media::MediaException &e )
    {
      ZYPP_CAUGHT(e);
      // TranslatorExplanation '%s' is an URL
      RepoException enew(str::form( _("Error trying to read from '%s'"), url.asString().c_str() ));
      enew.remember(e);
      ZYPP_THROW(enew);
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      // TranslatorExplanation '%s' is an URL
      Exception enew(str::form( _("Unknown error reading from '%s'"), url.asString().c_str() ));
      enew.remember(e);
      ZYPP_THROW(enew);
    }

    return repo::ServiceType::NONE;
  }

  ////////////////////////////////////////////////////////////////////////////

  std::ostream & operator<<( std::ostream & str, const RepoManager & obj )
  {
    return str << *obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
