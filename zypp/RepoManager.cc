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
#include "zypp/parser/RepoindexFileReader.h"
#include "zypp/repo/yum/Downloader.h"
#include "zypp/repo/susetags/Downloader.h"
#include "zypp/parser/plaindir/RepoParser.h"

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
          mediamanager.attachDesiredMedia( _mid );
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
    if ( filesystem::readdir( entries, Pathname(dir), false ) != 0 )
      ZYPP_THROW(Exception("failed to read directory"));

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
  }

  inline void assert_alias( const ServiceInfo & info )
  {
    if ( info.alias().empty() )
      ZYPP_THROW( ServiceNoAliasException() );
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
   * \short Calculates the raw metadata cache path for a repository, this is
   * inside the raw cache dir, plus the path where the metadata is.
   *
   * It should be different only for repositories that are not in the root of
   * the media.
   * for example /var/cache/zypp/alias/addondir
   */
  inline Pathname rawmetadata_path_for_repoinfo( const RepoManagerOptions &opt, const RepoInfo &info )
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
      ZYPP_THROW( Exception( "Can't open " + servfile.asString() ) );
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
      if ( filesystem::readdir( entries, Pathname(dir), false ) != 0 )
          ZYPP_THROW(Exception("failed to read directory"));

      //str::regex allowedServiceExt("^\\.service(_[0-9]+)?$");
      for_(it, entries.begin(), entries.end() )
      {
        parser::ServiceFileReader(*it, ServiceCollector(services));
      }
    }
  }

  void RepoManager::Impl::init_knownRepositories()
  {
    MIL << "start construct known repos" << endl;

    if ( PathInfo(options.knownReposPath).isExist() )
    {
      RepoInfoList repol = repositories_in_dir(options.knownReposPath);
      for ( RepoInfoList::iterator it = repol.begin();
            it != repol.end();
            ++it )
      {
        // set the metadata path for the repo
        Pathname metadata_path = rawmetadata_path_for_repoinfo(options, (*it));
        (*it).setMetadataPath(metadata_path);

	// set the downloaded packages path for the repo
	Pathname packages_path = packagescache_path_for_repoinfo(options, (*it));
	(*it).setPackagesPath(packages_path);

        repos.insert(*it);
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

  ////////////////////////////////////////////////////////////////////////////

  Pathname RepoManager::metadataPath( const RepoInfo &info ) const
  {
    return rawmetadata_path_for_repoinfo(_pimpl->options, info );
  }

  Pathname RepoManager::packagesPath( const RepoInfo &info ) const
  {
    return packagescache_path_for_repoinfo(_pimpl->options, info );
  }

  ////////////////////////////////////////////////////////////////////////////

  RepoStatus RepoManager::metadataStatus( const RepoInfo &info ) const
  {
    Pathname rawpath = rawmetadata_path_for_repoinfo( _pimpl->options, info );
    Pathname mediarootpath = rawcache_path_for_repoinfo( _pimpl->options, info );
    RepoType repokind = info.type();
    RepoStatus status;

    switch ( repokind.toEnum() )
    {
      case RepoType::NONE_e:
      // unknown, probe the local metadata
        repokind = probe(rawpath.asUrl());
      break;
      default:
      break;
    }

    switch ( repokind.toEnum() )
    {
      case RepoType::RPMMD_e :
      {
        status = RepoStatus( rawpath + "/repodata/repomd.xml");
      }
      break;

      case RepoType::YAST2_e :
      {
        status = RepoStatus( rawpath + "/content") && (RepoStatus( mediarootpath + "/media.1/media"));
      }
      break;

      case RepoType::RPMPLAINDIR_e :
      {
        if ( PathInfo(Pathname(rawpath + "/cookie")).isExist() )
          status = RepoStatus( rawpath + "/cookie");
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
    Pathname rawpath = rawmetadata_path_for_repoinfo( _pimpl->options, info );

    RepoType repokind = info.type();
    if ( repokind.toEnum() == RepoType::NONE_e )
      // unknown, probe the local metadata
      repokind = probe(rawpath.asUrl());
    // if still unknown, just return
    if (repokind == RepoType::NONE_e)
      return;

    Pathname p;
    switch ( repokind.toEnum() )
    {
      case RepoType::RPMMD_e :
        p = Pathname(rawpath + "/repodata/repomd.xml");
        break;

      case RepoType::YAST2_e :
        p = Pathname(rawpath + "/content");
        break;

      case RepoType::RPMPLAINDIR_e :
        p = Pathname(rawpath + "/cookie");
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

      repo::RepoType repokind = info.type();

      // if the type is unknown, try probing.
      switch ( repokind.toEnum() )
      {
        case RepoType::NONE_e:
          // unknown, probe it
          repokind = probe(url);
        break;
        default:
        break;
      }

      Pathname rawpath = rawmetadata_path_for_repoinfo( _pimpl->options, info );
      filesystem::assert_dir(rawpath);
      oldstatus = metadataStatus(info);

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

        if (diff < ZConfig::instance().repo_refresh_delay())
        {
          MIL << "Repository '" << info.alias()
              << "' has been refreshed less than repo.refresh.delay ("
              << ZConfig::instance().repo_refresh_delay()
              << ") minutes ago. Advising to skip refresh" << endl;
          return REPO_CHECK_DELAYED;
        }
      }

      // create temp dir as sibling of rawpath
      filesystem::TmpDir tmpdir( filesystem::TmpDir::makeSibling( rawpath ) );

      if ( ( repokind.toEnum() == RepoType::RPMMD_e ) ||
           ( repokind.toEnum() == RepoType::YAST2_e ) )
      {
        MediaSetAccess media(url);
        shared_ptr<repo::Downloader> downloader_ptr;

        if ( repokind.toEnum() == RepoType::RPMMD_e )
          downloader_ptr.reset(new yum::Downloader(info));
        else
          downloader_ptr.reset( new susetags::Downloader(info));

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
        RepoStatus newstatus = parser::plaindir::dirStatus(media.getPathName());
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
    RepoException rexception(_("Valid metadata not found at specified URL(s)"));

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
            repokind = probe(*it);

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

        Pathname rawpath = rawmetadata_path_for_repoinfo( _pimpl->options, info );
        filesystem::assert_dir(rawpath);

        // create temp dir as sibling of rawpath
        filesystem::TmpDir tmpdir( filesystem::TmpDir::makeSibling( rawpath ) );

        if ( ( repokind.toEnum() == RepoType::RPMMD_e ) ||
             ( repokind.toEnum() == RepoType::YAST2_e ) )
        {
          MediaSetAccess media(url);
          shared_ptr<repo::Downloader> downloader_ptr;

          MIL << "Creating downloader for [ " << info.name() << " ]" << endl;

          if ( repokind.toEnum() == RepoType::RPMMD_e )
            downloader_ptr.reset(new yum::Downloader(info));
          else
            downloader_ptr.reset( new susetags::Downloader(info) );

          /**
           * Given a downloader, sets the other repos raw metadata
           * path as cache paths for the fetcher, so if another
           * repo has the same file, it will not download it
           * but copy it from the other repository
           */
          for_( it, repoBegin(), repoEnd() )
          {
            Pathname cachepath(rawmetadata_path_for_repoinfo( _pimpl->options, *it ));
            if ( PathInfo(cachepath).isExist() )
              downloader_ptr->addCachePath(cachepath);
          }

          downloader_ptr->download( media, tmpdir.path());
        }
        else if ( repokind.toEnum() == RepoType::RPMPLAINDIR_e )
        {
          MediaMounter media( url );
          RepoStatus newstatus = parser::plaindir::dirStatus(media.getPathName());

          std::ofstream file(( tmpdir.path() + "/cookie").c_str());
          if (!file) {
            ZYPP_THROW (Exception( "Can't open " + tmpdir.path().asString() + "/cookie" ) );
          }
          file << url << endl;
          file << newstatus.checksum() << endl;

          file.close();
        }
        else
        {
          ZYPP_THROW(RepoUnknownTypeException());
        }

        // ok we have the metadata, now exchange
        // the contents

        filesystem::TmpDir oldmetadata( filesystem::TmpDir::makeSibling( rawpath ) );
        filesystem::rename( rawpath, oldmetadata.path() );
        // move the just downloaded there
        filesystem::rename( tmpdir.path(), rawpath );

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
    Pathname rawpath = rawmetadata_path_for_repoinfo(_pimpl->options, info);

    filesystem::assert_dir(_pimpl->options.repoCachePath);
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
    progress.name(str::form(_("Building repository '%s' cache"), info.name().c_str()));
    progress.toMin();

    if (needs_cleaning)
    {
      cleanCache(info);
    }

    MIL << info.alias() << " building cache..." << endl;

    Pathname base = solv_path_for_repoinfo( _pimpl->options, info);
    filesystem::assert_dir(base);
    Pathname solvfile = base / "solv";

    // do we have type?
    repo::RepoType repokind = info.type();

    // if the type is unknown, try probing.
    switch ( repokind.toEnum() )
    {
      case RepoType::NONE_e:
        // unknown, probe the local metadata
        repokind = probe(rawpath.asUrl());
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
          cmd.push_back( forPlainDirs->getPathName().c_str() );
        }
        else
          cmd.push_back( rawpath.asString() );

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
          RepoException ex(str::form("Failed to cache repo (%d).", ret));
          ex.remember( errdetail );
          ZYPP_THROW(ex);
        }

        // We keep it.
        guard.resetDispose();
      }
      break;
      default:
        ZYPP_THROW(RepoUnknownTypeException("Unhandled repository type"));
      break;
    }
    // update timestamp and checksum
    setCacheStatus(info, raw_metadata_status);
    MIL << "Commit cache.." << endl;
    progress.toMax();
  }

  ////////////////////////////////////////////////////////////////////////////

  repo::RepoType RepoManager::probe( const Url &url ) const
  {
    MIL << "going to probe the type of the repo " << endl;

    if ( url.getScheme() == "dir" && ! PathInfo( url.getPathName() ).isDir() )
    {
      // Handle non existing local directory in advance, as
      // MediaSetAccess does not support it.
      MIL << "Probed type NONE (not exists) at " << url << endl;
      return repo::RepoType::NONE;
    }

    // prepare exception to be thrown if the type could not be determined
    // due to a media exception. We can't throw right away, because of some
    // problems with proxy servers returning an incorrect error
    // on ftp file-not-found(bnc #335906). Instead we'll check another types
    // before throwing.
    RepoException enew("Error trying to read from " + url.asString());
    bool gotMediaException = false;
    try
    {
      MediaSetAccess access(url);
      try
      {
        if ( access.doesFileExist("/repodata/repomd.xml") )
        {
          MIL << "Probed type RPMMD at " << url << endl;
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
        if ( access.doesFileExist("/content") )
        {
          MIL << "Probed type YAST2 at " << url << endl;
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
      if ( ! media::MediaManager::downloads(url) )
      {
        MediaMounter media( url );
        if ( PathInfo(media.getPathName()).isDir() )
        {
          // allow empty dirs for now
          MIL << "Probed type RPMPLAINDIR at " << url << endl;
          return repo::RepoType::RPMPLAINDIR;
        }
      }
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      Exception enew("Unknown error reading from " + url.asString());
      enew.remember(e);
      ZYPP_THROW(enew);
    }

    if (gotMediaException)
      ZYPP_THROW(enew);

    MIL << "Probed type NONE at " << url << endl;
    return repo::RepoType::NONE;
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::cleanCache( const RepoInfo &info,
                                const ProgressData::ReceiverFnc & progressrcv )
  {
    ProgressData progress(100);
    progress.sendTo(progressrcv);
    progress.toMin();

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

    try
    {
      sat::Pool::instance().addRepoSolv( solvfile, info );
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
    progress.name(str::form(_("Adding repository '%s'"), info.name().c_str()));
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
      probedtype = probe(*tosave.baseUrlsBegin());
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
    if (!file) {
      ZYPP_THROW (Exception( "Can't open " + repofile.asString() ) );
    }

    tosave.dumpAsIniOn(file);
    tosave.setFilepath(repofile);
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
      ZYPP_THROW(RepoException("Invalid repo file name at " + url.asString() ));

    // assert the directory exists
    filesystem::assert_dir(_pimpl->options.knownReposPath);

    Pathname repofile = _pimpl->generateNonExistingName(_pimpl->options.knownReposPath, filename);
    // now we have a filename that does not exists
    MIL << "Saving " << repos.size() << " repo" << ( repos.size() ? "s" : "" ) << " in " << repofile << endl;

    std::ofstream file(repofile.c_str());
    if (!file) {
      ZYPP_THROW (Exception( "Can't open " + repofile.asString() ) );
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
    progress.name(str::form(_("Removing repository '%s'"), info.name().c_str()));

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
        ZYPP_THROW(RepoException("Can't figure where the repo is stored"));
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
            ZYPP_THROW(RepoException("Can't delete " + todelete.filepath().asString()));
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
          if (!file) {
            //ZYPP_THROW (Exception( "Can't open " + tmp.path().asString() ) );
            ZYPP_THROW (Exception( "Can't open " + todelete.filepath().asString() ) );
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
                                      const RepoInfo & newinfo,
                                      const ProgressData::ReceiverFnc & progressrcv )
  {
    RepoInfo toedit = getRepositoryInfo(alias);

    // check if the new alias already exists when renaming the repo
    if (alias != newinfo.alias())
    {
      for_( it, repoBegin(), repoEnd() )
      {
        if ( newinfo.alias() == (*it).alias() )
          ZYPP_THROW(RepoAlreadyExistsException(newinfo));
      }
    }

    if (toedit.filepath().empty())
    {
      ZYPP_THROW(RepoException("Can't figure where the repo is stored"));
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
      if (!file) {
        //ZYPP_THROW (Exception( "Can't open " + tmp.path().asString() ) );
        ZYPP_THROW (Exception( "Can't open " + toedit.filepath().asString() ) );
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
      ZYPP_THROW(RepoException("Can't figure where the service is stored"));
    }

    ServiceSet tmpSet;
    parser::ServiceFileReader( location, ServiceCollector(tmpSet) );

    // only one service definition in the file
    if ( tmpSet.size() == 1 )
    {
      if ( filesystem::unlink(location) != 0 )
      {
        ZYPP_THROW(RepoException("Can't delete " + location.asString()));
      }
      MIL << alias << " sucessfully deleted." << endl;
    }
    else
    {
      filesystem::assert_dir(location.dirname());

      std::ofstream file(location.c_str());
      if( file.fail() )
        ZYPP_THROW(Exception("failed open file to write"));

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

      refreshService(*it);
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
    MIL << "going to refresh service '" << service.alias() << "', url: "<< service.url() << endl;

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

    // download the repo index file
    media::MediaManager mediamanager;
    media::MediaAccessId mid = mediamanager.open( service.url() );
    mediamanager.attachDesiredMedia( mid );
    mediamanager.provideFile( mid, "repo/repoindex.xml" );
    Pathname path = mediamanager.localPath(mid, "repo/repoindex.xml" );

    // get target distro identifier
    std::string servicesTargetDistro = _pimpl->options.servicesTargetDistro;
    if ( servicesTargetDistro.empty() && getZYpp()->getTarget() )
      servicesTargetDistro = getZYpp()->target()->targetDistribution();
    DBG << "servicesTargetDistro: " << servicesTargetDistro << endl;

    // parse it
    RepoCollector collector(servicesTargetDistro);
    parser::RepoindexFileReader reader( path, bind( &RepoCollector::collect, &collector, _1 ) );
    mediamanager.release( mid );
    mediamanager.close( mid );


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

        // Make sure the service repo is created with the
        // appropriate enable and autorefresh true.
        it->setEnabled( beEnabled );
        it->setAutorefresh( true );

        // At that point check whether a repo with the same alias
        // exists outside this service. Maybe forcefully re-alias
        // the existing repo?
        addRepository( *it );

        // save repo credentials
        // ma@: task for modifyRepository?
      }
      else
      {
        // ==> an exising repo to check
        bool oldRepoModified = false;

        if ( beEnabled )
        {
          if ( ! oldRepo->enabled() )
          {
            oldRepo->setEnabled( true );
            oldRepoModified = true;
          }
        }
        else if ( beDisabled )
        {
          if ( oldRepo->enabled() )
          {
            oldRepo->setEnabled( false );
            oldRepoModified = true;
          }
        }

#warning also check changed URL due to PATH/URL change in service, but ignore ?credentials param!
// ma@: task for modifyRepository?

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
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::modifyService(const std::string & oldAlias, const ServiceInfo & service)
  {
    MIL << "Going to modify service " << oldAlias << endl;

    const ServiceInfo & oldService = getService(oldAlias);

    Pathname location = oldService.filepath();
    if( location.empty() )
    {
      ZYPP_THROW(RepoException(
          "Cannot figure out where the service file is stored."));
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
      RepoException enew("Error trying to read from " + url.asString());
      enew.remember(e);
      ZYPP_THROW(enew);
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      Exception enew("Unknown error reading from " + url.asString());
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
