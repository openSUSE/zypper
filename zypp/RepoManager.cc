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
#include "zypp/base/DefaultIntegral.h"
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
#include "zypp/repo/PluginServices.h"

#include "zypp/Target.h" // for Target::targetDistribution() for repo index services
#include "zypp/ZYppFactory.h" // to get the Target from ZYpp instance
#include "zypp/HistoryLog.h" // to write history :O)

#include "zypp/ZYppCallbacks.h"

#include "sat/Pool.h"

using std::endl;
using std::string;
using namespace zypp::repo;

#define OPT_PROGRESS const ProgressData::ReceiverFnc & = ProgressData::ReceiverFnc()

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
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
    ///////////////////////////////////////////////////////////////////

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


    /** \short Generate a related filename from a repo/service infos alias */
    inline std::string filenameFromAlias( const std::string & alias_r, const std::string & stem_r )
    {
      std::string filename( alias_r );
      // replace slashes with underscores
      str::replaceAll( filename, "/", "_" );

      filename = Pathname(filename).extend("."+stem_r).asString();
      MIL << "generating filename for " << stem_r << " [" << alias_r << "] : '" << filename << "'" << endl;
      return filename;
    }

    /**
     * \short Simple callback to collect the results
     *
     * Classes like RepoFileReader call the callback
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
            << "Skipping repository meant for '" << repo.targetDistribution()
            << "' distribution (current distro is '"
            << targetDistro << "')." << endl;

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
    std::list<RepoInfo> repositories_in_file( const Pathname & file )
    {
      MIL << "repo file: " << file << endl;
      RepoCollector collector;
      parser::RepoFileReader parser( file, bind( &RepoCollector::collect, &collector, _1 ) );
      return std::move(collector.repos);
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
    std::list<RepoInfo> repositories_in_dir( const Pathname &dir )
    {
      MIL << "directory " << dir << endl;
      std::list<RepoInfo> repos;
      bool nonroot( geteuid() != 0 );
      if ( nonroot && ! PathInfo(dir).userMayRX() )
      {
	JobReport::warning( formatNAC(_("Cannot read repo directory '%1%': Permission denied")) % dir );
      }
      else
      {
	std::list<Pathname> entries;
	if ( filesystem::readdir( entries, dir, false ) != 0 )
	{
	  // TranslatorExplanation '%s' is a pathname
	  ZYPP_THROW(Exception(str::form(_("Failed to read directory '%s'"), dir.c_str())));
	}

	str::regex allowedRepoExt("^\\.repo(_[0-9]+)?$");
	for ( std::list<Pathname>::const_iterator it = entries.begin(); it != entries.end(); ++it )
	{
	  if ( str::regex_match(it->extension(), allowedRepoExt) )
	  {
	    if ( nonroot && ! PathInfo(*it).userMayR() )
	    {
	      JobReport::warning( formatNAC(_("Cannot read repo file '%1%': Permission denied")) % *it );
	    }
	    else
	    {
	      const std::list<RepoInfo> & tmp( repositories_in_file( *it ) );
	      repos.insert( repos.end(), tmp.begin(), tmp.end() );
	    }
	  }
	}
      }
      return repos;
    }

    ////////////////////////////////////////////////////////////////////////////

    inline void assert_alias( const RepoInfo & info )
    {
      if ( info.alias().empty() )
	ZYPP_THROW( RepoNoAliasException( info ) );
      // bnc #473834. Maybe we can match the alias against a regex to define
      // and check for valid aliases
      if ( info.alias()[0] == '.')
	ZYPP_THROW(RepoInvalidAliasException(
	  info, _("Repository alias cannot start with dot.")));
    }

    inline void assert_alias( const ServiceInfo & info )
    {
      if ( info.alias().empty() )
	ZYPP_THROW( ServiceNoAliasException( info ) );
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

  } // namespace
  ///////////////////////////////////////////////////////////////////

  std::list<RepoInfo> readRepoFile( const Url & repo_file )
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

  ///////////////////////////////////////////////////////////////////
  //
  //	class RepoManagerOptions
  //
  ////////////////////////////////////////////////////////////////////

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

  std:: ostream & operator<<( std::ostream & str, const RepoManagerOptions & obj )
  {
#define OUTS(X) str << "  " #X "\t" << obj.X << endl
    str << "RepoManagerOptions (" << obj.rootDir << ") {" << endl;
    OUTS( repoRawCachePath );
    OUTS( repoSolvCachePath );
    OUTS( repoPackagesCachePath );
    OUTS( knownReposPath );
    OUTS( knownServicesPath );
    OUTS( pluginsPath );
    str << "}" << endl;
#undef OUTS
    return str;
  }

  ///////////////////////////////////////////////////////////////////
  /// \class RepoManager::Impl
  /// \brief RepoManager implementation.
  ///
  ///////////////////////////////////////////////////////////////////
  struct RepoManager::Impl
  {
  public:
    Impl( const RepoManagerOptions &opt )
      : _options(opt)
    {
      init_knownServices();
      init_knownRepositories();
    }

    ~Impl()
    {
      // trigger appdata refresh if some repos change
      if ( _reposDirty && geteuid() == 0 && ( _options.rootDir.empty() || _options.rootDir == "/" ) )
      {
	try {
	  std::list<Pathname> entries;
	  filesystem::readdir( entries, _options.pluginsPath/"appdata", false );
	  if ( ! entries.empty() )
	  {
	    ExternalProgram::Arguments cmd;
	    cmd.push_back( "<" );		// discard stdin
	    cmd.push_back( ">" );		// discard stdout
	    cmd.push_back( "PROGRAM" );		// [2] - fix index below if changing!
	    for ( const auto & rinfo : repos() )
	    {
	      if ( ! rinfo.enabled() )
		continue;
	      cmd.push_back( "-R" );
	      cmd.push_back( rinfo.alias() );
	      cmd.push_back( "-t" );
	      cmd.push_back( rinfo.type().asString() );
	      cmd.push_back( "-p" );
	      cmd.push_back( rinfo.metadataPath().asString() );
	    }

	    for_( it, entries.begin(), entries.end() )
	    {
	      PathInfo pi( *it );
	      //DBG << "/tmp/xx ->" << pi << endl;
	      if ( pi.isFile() && pi.userMayRX() )
	      {
		// trigger plugin
		cmd[2] = pi.asString();		// [2] - PROGRAM
		ExternalProgram prog( cmd, ExternalProgram::Stderr_To_Stdout );
	      }
	    }
	  }
	}
	catch (...) {}	// no throw in dtor
      }
    }

  public:
    bool repoEmpty() const		{ return repos().empty(); }
    RepoSizeType repoSize() const	{ return repos().size(); }
    RepoConstIterator repoBegin() const	{ return repos().begin(); }
    RepoConstIterator repoEnd() const	{ return repos().end(); }

    bool hasRepo( const std::string & alias ) const
    { return foundAliasIn( alias, repos() ); }

    RepoInfo getRepo( const std::string & alias ) const
    {
      RepoConstIterator it( findAlias( alias, repos() ) );
      return it == repos().end() ? RepoInfo::noRepo : *it;
    }

  public:
    Pathname metadataPath( const RepoInfo & info ) const
    { return rawcache_path_for_repoinfo( _options, info ); }

    Pathname packagesPath( const RepoInfo & info ) const
    { return packagescache_path_for_repoinfo( _options, info ); }

    RepoStatus metadataStatus( const RepoInfo & info ) const;

    RefreshCheckStatus checkIfToRefreshMetadata( const RepoInfo & info, const Url & url, RawMetadataRefreshPolicy policy );

    void refreshMetadata( const RepoInfo & info, RawMetadataRefreshPolicy policy, OPT_PROGRESS );

    void cleanMetadata( const RepoInfo & info, OPT_PROGRESS );

    void cleanPackages( const RepoInfo & info, OPT_PROGRESS );

    void buildCache( const RepoInfo & info, CacheBuildPolicy policy, OPT_PROGRESS );

    repo::RepoType probe( const Url & url, const Pathname & path = Pathname() ) const;
    repo::RepoType probeCache( const Pathname & path_r ) const;

    void cleanCacheDirGarbage( OPT_PROGRESS );

    void cleanCache( const RepoInfo & info, OPT_PROGRESS );

    bool isCached( const RepoInfo & info ) const
    { return PathInfo(solv_path_for_repoinfo( _options, info ) / "solv").isExist(); }

    RepoStatus cacheStatus( const RepoInfo & info ) const
    { return RepoStatus::fromCookieFile(solv_path_for_repoinfo(_options, info) / "cookie"); }

    void loadFromCache( const RepoInfo & info, OPT_PROGRESS );

    void addRepository( const RepoInfo & info, OPT_PROGRESS );

    void addRepositories( const Url & url, OPT_PROGRESS );

    void removeRepository( const RepoInfo & info, OPT_PROGRESS );

    void modifyRepository( const std::string & alias, const RepoInfo & newinfo_r, OPT_PROGRESS );

    RepoInfo getRepositoryInfo( const std::string & alias, OPT_PROGRESS );
    RepoInfo getRepositoryInfo( const Url & url, const url::ViewOption & urlview, OPT_PROGRESS );

  public:
    bool serviceEmpty() const			{ return _services.empty(); }
    ServiceSizeType serviceSize() const		{ return _services.size(); }
    ServiceConstIterator serviceBegin() const	{ return _services.begin(); }
    ServiceConstIterator serviceEnd() const	{ return _services.end(); }

    bool hasService( const std::string & alias ) const
    { return foundAliasIn( alias, _services ); }

    ServiceInfo getService( const std::string & alias ) const
    {
      ServiceConstIterator it( findAlias( alias, _services ) );
      return it == _services.end() ? ServiceInfo::noService : *it;
    }

  public:
    void addService( const ServiceInfo & service );
    void addService( const std::string & alias, const Url & url )
    { addService( ServiceInfo( alias, url ) ); }

    void removeService( const std::string & alias );
    void removeService( const ServiceInfo & service )
    { removeService( service.alias() ); }

    void refreshServices( const RefreshServiceOptions & options_r );

    void refreshService( const std::string & alias, const RefreshServiceOptions & options_r );
    void refreshService( const ServiceInfo & service, const RefreshServiceOptions & options_r )
    {  refreshService( service.alias(), options_r ); }

    void modifyService( const std::string & oldAlias, const ServiceInfo & newService );

    repo::ServiceType probeService( const Url & url ) const;

  private:
    void saveService( ServiceInfo & service ) const;

    Pathname generateNonExistingName( const Pathname & dir, const std::string & basefilename ) const;

    std::string generateFilename( const RepoInfo & info ) const
    { return filenameFromAlias( info.alias(), "repo" ); }

    std::string generateFilename( const ServiceInfo & info ) const
    { return filenameFromAlias( info.alias(), "service" ); }

    void setCacheStatus( const RepoInfo & info, const RepoStatus & status )
    {
      Pathname base = solv_path_for_repoinfo( _options, info );
      filesystem::assert_dir(base);
      status.saveToCookieFile( base / "cookie" );
    }

    void touchIndexFile( const RepoInfo & info );

    template<typename OutputIterator>
    void getRepositoriesInService( const std::string & alias, OutputIterator out ) const
    {
      MatchServiceAlias filter( alias );
      std::copy( boost::make_filter_iterator( filter, repos().begin(), repos().end() ),
                 boost::make_filter_iterator( filter, repos().end(), repos().end() ),
                 out);
    }

  private:
    void init_knownServices();
    void init_knownRepositories();

    const RepoSet & repos() const { return _reposX; }
    RepoSet & reposManip()        { if ( ! _reposDirty ) _reposDirty = true; return _reposX; }

  private:
    RepoManagerOptions	_options;
    RepoSet 		_reposX;
    ServiceSet		_services;

    DefaultIntegral<bool,false> _reposDirty;

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoManager::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const RepoManager::Impl & obj )
  { return str << "RepoManager::Impl"; }

  ///////////////////////////////////////////////////////////////////

  void RepoManager::Impl::saveService( ServiceInfo & service ) const
  {
    filesystem::assert_dir( _options.knownServicesPath );
    Pathname servfile = generateNonExistingName( _options.knownServicesPath,
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
      ++counter;
    }
    return dir + Pathname(final_filename);
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::Impl::init_knownServices()
  {
    Pathname dir = _options.knownServicesPath;
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
        parser::ServiceFileReader(*it, ServiceCollector(_services));
      }
    }

    repo::PluginServices(_options.pluginsPath/"services", ServiceCollector(_services));
  }

  ///////////////////////////////////////////////////////////////////
  namespace {
    /** Delete \a cachePath_r subdirs not matching known aliases in \a repoEscAliases_r (must be sorted!)
     * \note bnc#891515: Auto-cleanup only zypp.conf default locations. Otherwise
     * we'd need some magic file to identify zypp cache directories. Without this
     * we may easily remove user data (zypper --pkg-cache-dir . download ...)
     */
    inline void cleanupNonRepoMetadtaFolders( const Pathname & cachePath_r,
					      const Pathname & defaultCachePath_r,
					      const std::list<std::string> & repoEscAliases_r )
    {
      if ( cachePath_r != defaultCachePath_r )
	return;

      std::list<std::string> entries;
      if ( filesystem::readdir( entries, cachePath_r, false ) == 0 )
      {
	entries.sort();
	std::set<std::string> oldfiles;
	set_difference( entries.begin(), entries.end(), repoEscAliases_r.begin(), repoEscAliases_r.end(),
			std::inserter( oldfiles, oldfiles.end() ) );
	for ( const std::string & old : oldfiles )
	{
	  if ( old == Repository::systemRepoAlias() )	// don't remove the @System solv file
	    continue;
	  filesystem::recursive_rmdir( cachePath_r / old );
	}
      }
    }
  } // namespace
  ///////////////////////////////////////////////////////////////////
  void RepoManager::Impl::init_knownRepositories()
  {
    MIL << "start construct known repos" << endl;

    if ( PathInfo(_options.knownReposPath).isExist() )
    {
      std::list<std::string> repoEscAliases;
      std::list<RepoInfo> orphanedRepos;
      for ( RepoInfo & repoInfo : repositories_in_dir(_options.knownReposPath) )
      {
        // set the metadata path for the repo
        repoInfo.setMetadataPath( rawcache_path_for_repoinfo(_options, repoInfo) );
	// set the downloaded packages path for the repo
	repoInfo.setPackagesPath( packagescache_path_for_repoinfo(_options, repoInfo) );
	// remember it
        _reposX.insert( repoInfo );	// direct access via _reposX in ctor! no reposManip.

	// detect orphaned repos belonging to a deleted service
	const std::string & serviceAlias( repoInfo.service() );
	if ( ! ( serviceAlias.empty() || hasService( serviceAlias ) ) )
	{
	  WAR << "Schedule orphaned service repo for deletion: " << repoInfo << endl;
	  orphanedRepos.push_back( repoInfo );
	  continue;	// don't remember it in repoEscAliases
	}

        repoEscAliases.push_back(repoInfo.escaped_alias());
      }

      // Cleanup orphanded service repos:
      if ( ! orphanedRepos.empty() )
      {
	for ( auto & repoInfo : orphanedRepos )
	{
	  MIL << "Delete orphaned service repo " << repoInfo.alias() << endl;
	  // translators: Cleanup a repository previously owned by a meanwhile unknown (deleted) service.
	  //   %1% = service name
	  //   %2% = repository name
	  JobReport::warning( formatNAC(_("Unknown service '%1%': Removing orphaned service repository '%2%'" ))
			      % repoInfo.service()
			      % repoInfo.alias() );
	  try {
	    removeRepository( repoInfo );
	  }
	  catch ( const Exception & caugth )
	  {
	    JobReport::error( caugth.asUserHistory() );
	  }
	}
      }

      // delete metadata folders without corresponding repo (e.g. old tmp directories)
      //
      // bnc#891515: Auto-cleanup only zypp.conf default locations. Otherwise
      // we'd need somemagic file to identify zypp cache directories. Without this
      // we may easily remove user data (zypper --pkg-cache-dir . download ...)
      repoEscAliases.sort();
      RepoManagerOptions defaultCache( _options.rootDir );
      cleanupNonRepoMetadtaFolders( _options.repoRawCachePath,		defaultCache.repoRawCachePath,		repoEscAliases );
      cleanupNonRepoMetadtaFolders( _options.repoSolvCachePath,		defaultCache.repoSolvCachePath,		repoEscAliases );
      cleanupNonRepoMetadtaFolders( _options.repoPackagesCachePath,	defaultCache.repoPackagesCachePath,	repoEscAliases );
    }
    MIL << "end construct known repos" << endl;
  }

  ///////////////////////////////////////////////////////////////////

  RepoStatus RepoManager::Impl::metadataStatus( const RepoInfo & info ) const
  {
    Pathname mediarootpath = rawcache_path_for_repoinfo( _options, info );
    Pathname productdatapath = rawproductdata_path_for_repoinfo( _options, info );

    RepoType repokind = info.type();
    // If unknown, probe the local metadata
    if ( repokind == RepoType::NONE )
      repokind = probeCache( productdatapath );

    RepoStatus status;
    switch ( repokind.toEnum() )
    {
      case RepoType::RPMMD_e :
        status = RepoStatus( productdatapath/"repodata/repomd.xml");
	break;

      case RepoType::YAST2_e :
        status = RepoStatus( productdatapath/"content" ) && RepoStatus( mediarootpath/"media.1/media" );
	break;

      case RepoType::RPMPLAINDIR_e :
	status = RepoStatus::fromCookieFile( productdatapath/"cookie" );
	break;

      case RepoType::NONE_e :
	// Return default RepoStatus in case of RepoType::NONE
	// indicating it should be created?
        // ZYPP_THROW(RepoUnknownTypeException());
	break;
    }
    return status;
  }


  void RepoManager::Impl::touchIndexFile( const RepoInfo & info )
  {
    Pathname productdatapath = rawproductdata_path_for_repoinfo( _options, info );

    RepoType repokind = info.type();
    if ( repokind.toEnum() == RepoType::NONE_e )
      // unknown, probe the local metadata
      repokind = probeCache( productdatapath );
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


  RepoManager::RefreshCheckStatus RepoManager::Impl::checkIfToRefreshMetadata( const RepoInfo & info, const Url & url, RawMetadataRefreshPolicy policy )
  {
    assert_alias(info);
    try
    {
      MIL << "Going to try to check whether refresh is needed for " << url << endl;

      // first check old (cached) metadata
      Pathname mediarootpath = rawcache_path_for_repoinfo( _options, info );
      filesystem::assert_dir( mediarootpath );
      RepoStatus oldstatus = metadataStatus( info );

      if ( oldstatus.empty() )
      {
        MIL << "No cached metadata, going to refresh" << endl;
        return REFRESH_NEEDED;
      }

      {
        if ( url.schemeIsVolatile() )
	{
	  MIL << "never refresh CD/DVD" << endl;
          return REPO_UP_TO_DATE;
	}
	if ( url.schemeIsLocal() )
	{
	  policy = RefreshIfNeededIgnoreDelay;
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

      repo::RepoType repokind = info.type();
      // if unknown: probe it
      if ( repokind == RepoType::NONE )
	repokind = probe( url, info.path() );

      // retrieve newstatus
      RepoStatus newstatus;
      switch ( repokind.toEnum() )
      {
	case RepoType::RPMMD_e:
	{
	  MediaSetAccess media( url );
	  newstatus = yum::Downloader( info, mediarootpath ).status( media );
	}
	break;

	case RepoType::YAST2_e:
	{
	  MediaSetAccess media( url );
	  newstatus = susetags::Downloader( info, mediarootpath ).status( media );
	}
	break;

	case RepoType::RPMPLAINDIR_e:
	  newstatus = RepoStatus( MediaMounter(url).getPathName(info.path()) );	// dir status
	  break;

	default:
	case RepoType::NONE_e:
	  ZYPP_THROW( RepoUnknownTypeException( info ) );
	  break;
      }

      // check status
      bool refresh = false;
      if ( oldstatus == newstatus )
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
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      ERR << "refresh check failed for " << url << endl;
      ZYPP_RETHROW(e);
    }

    return REFRESH_NEEDED; // default
  }


  void RepoManager::Impl::refreshMetadata( const RepoInfo & info, RawMetadataRefreshPolicy policy, const ProgressData::ReceiverFnc & progress )
  {
    assert_alias(info);
    assert_urls(info);

    // we will throw this later if no URL checks out fine
    RepoException rexception( info, PL_("Valid metadata not found at specified URL",
					"Valid metadata not found at specified URLs",
					info.baseUrlsSize() ) );

    // Suppress (interactive) media::MediaChangeReport if we in have multiple basurls (>1)
    media::ScopedDisableMediaChangeReport guard( info.baseUrlsSize() > 1 );

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
	if ( repokind == RepoType::NONE )
	{
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
	}

        Pathname mediarootpath = rawcache_path_for_repoinfo( _options, info );
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
            Pathname cachepath(rawcache_path_for_repoinfo( _options, *it ));
            if ( PathInfo(cachepath).isExist() )
              downloader_ptr->addCachePath(cachepath);
          }

          downloader_ptr->download( media, tmpdir.path() );
        }
        else if ( repokind.toEnum() == RepoType::RPMPLAINDIR_e )
        {
          MediaMounter media( url );
          RepoStatus newstatus = RepoStatus( media.getPathName( info.path() ) );	// dir status

          Pathname productpath( tmpdir.path() / info.path() );
          filesystem::assert_dir( productpath );
	  newstatus.saveToCookieFile( productpath/"cookie" );
        }
        else
        {
          ZYPP_THROW(RepoUnknownTypeException( info ));
        }

        // ok we have the metadata, now exchange
        // the contents
	filesystem::exchange( tmpdir.path(), mediarootpath );
	reposManip();	// remember to trigger appdata refresh

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

  void RepoManager::Impl::cleanMetadata( const RepoInfo & info, const ProgressData::ReceiverFnc & progressfnc )
  {
    ProgressData progress(100);
    progress.sendTo(progressfnc);

    filesystem::recursive_rmdir(rawcache_path_for_repoinfo(_options, info));
    progress.toMax();
  }


  void RepoManager::Impl::cleanPackages( const RepoInfo & info, const ProgressData::ReceiverFnc & progressfnc )
  {
    ProgressData progress(100);
    progress.sendTo(progressfnc);

    filesystem::recursive_rmdir(packagescache_path_for_repoinfo(_options, info));
    progress.toMax();
  }


  void RepoManager::Impl::buildCache( const RepoInfo & info, CacheBuildPolicy policy, const ProgressData::ReceiverFnc & progressrcv )
  {
    assert_alias(info);
    Pathname mediarootpath = rawcache_path_for_repoinfo( _options, info );
    Pathname productdatapath = rawproductdata_path_for_repoinfo( _options, info );

    if( filesystem::assert_dir(_options.repoCachePath) )
    {
      Exception ex(str::form( _("Can't create %s"), _options.repoCachePath.c_str()) );
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

      if ( cache_status == raw_metadata_status )
      {
        MIL << info.alias() << " cache is up to date with metadata." << endl;
        if ( policy == BuildIfNeeded )
	{
	  // On the fly add missing solv.idx files for bash completion.
	  const Pathname & base = solv_path_for_repoinfo( _options, info);
	  if ( ! PathInfo(base/"solv.idx").isExist() )
	    sat::updateSolvFileIndex( base/"solv" );

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

    Pathname base = solv_path_for_repoinfo( _options, info);

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
        repokind = probeCache( productdatapath );
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
	cmd.push_back( "-X" );	// autogenerate pattern from pattern-package

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
	sat::updateSolvFileIndex( solvfile );	// content digest for zypper bash completion
      }
      break;
      default:
        ZYPP_THROW(RepoUnknownTypeException( info, _("Unhandled repository type") ));
      break;
    }
    // update timestamp and checksum
    setCacheStatus(info, raw_metadata_status);
    MIL << "Commit cache.." << endl;
    progress.toMax();
  }

  ////////////////////////////////////////////////////////////////////////////


  /** Probe the metadata type of a repository located at \c url.
   * Urls here may be rewritten by \ref MediaSetAccess to reflect the correct media number.
   *
   * \note Metadata in local cache directories must be probed using \ref probeCache as
   * a cache path must not be rewritten (bnc#946129)
   */
  repo::RepoType RepoManager::Impl::probe( const Url & url, const Pathname & path  ) const
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

  /** Probe Metadata in a local cache directory
   *
   * \note Metadata in local cache directories must not be probed using \ref probe as
   * a cache path must not be rewritten (bnc#946129)
   */
  repo::RepoType RepoManager::Impl::probeCache( const Pathname & path_r ) const
  {
    MIL << "going to probe the cached repo at " << path_r << endl;

    repo::RepoType ret = repo::RepoType::NONE;

    if ( PathInfo(path_r/"/repodata/repomd.xml").isFile() )
    { ret = repo::RepoType::RPMMD; }
    else if ( PathInfo(path_r/"/content").isFile() )
    { ret = repo::RepoType::YAST2; }
    else if ( PathInfo(path_r).isDir() )
    { ret = repo::RepoType::RPMPLAINDIR; }

    MIL << "Probed cached type " << ret << " at " << path_r << endl;
    return ret;
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::Impl::cleanCacheDirGarbage( const ProgressData::ReceiverFnc & progressrcv )
  {
    MIL << "Going to clean up garbage in cache dirs" << endl;

    ProgressData progress(300);
    progress.sendTo(progressrcv);
    progress.toMin();

    std::list<Pathname> cachedirs;
    cachedirs.push_back(_options.repoRawCachePath);
    cachedirs.push_back(_options.repoPackagesCachePath);
    cachedirs.push_back(_options.repoSolvCachePath);

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

          if ( ! found && ( Date::now()-PathInfo(*subdir).mtime() > Date::day ) )
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

  void RepoManager::Impl::cleanCache( const RepoInfo & info, const ProgressData::ReceiverFnc & progressrcv )
  {
    ProgressData progress(100);
    progress.sendTo(progressrcv);
    progress.toMin();

    MIL << "Removing raw metadata cache for " << info.alias() << endl;
    filesystem::recursive_rmdir(solv_path_for_repoinfo(_options, info));

    progress.toMax();
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::Impl::loadFromCache( const RepoInfo & info, const ProgressData::ReceiverFnc & progressrcv )
  {
    assert_alias(info);
    Pathname solvfile = solv_path_for_repoinfo(_options, info) / "solv";

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

  void RepoManager::Impl::addRepository( const RepoInfo & info, const ProgressData::ReceiverFnc & progressrcv )
  {
    assert_alias(info);

    ProgressData progress(100);
    callback::SendReport<ProgressReport> report;
    progress.sendTo( ProgressReportAdaptor( progressrcv, report ) );
    progress.name(str::form(_("Adding repository '%s'"), info.label().c_str()));
    progress.toMin();

    MIL << "Try adding repo " << info << endl;

    RepoInfo tosave = info;
    if ( repos().find(tosave) != repos().end() )
      ZYPP_THROW(RepoAlreadyExistsException(info));

    // check the first url for now
    if ( _options.probe )
    {
      DBG << "unknown repository type, probing" << endl;

      RepoType probedtype;
      probedtype = probe( *tosave.baseUrlsBegin(), info.path() );
      if ( tosave.baseUrlsSize() > 0 )
      {
        if ( probedtype == RepoType::NONE )
          ZYPP_THROW(RepoUnknownTypeException(info));
        else
          tosave.setType(probedtype);
      }
    }

    progress.set(50);

    // assert the directory exists
    filesystem::assert_dir(_options.knownReposPath);

    Pathname repofile = generateNonExistingName(
        _options.knownReposPath, generateFilename(tosave));
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
    reposManip().insert(tosave);

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
          media::CredManagerOptions(_options.rootDir) );

      for_(urlit, tosave.baseUrlsBegin(), tosave.baseUrlsEnd())
        if (urlit->hasCredentialsInAuthority())
          //! \todo use a method calling UI callbacks to ask where to save creds?
          cm.saveInUser(media::AuthData(*urlit));
    }

    HistoryLog().addRepository(tosave);

    progress.toMax();
    MIL << "done" << endl;
  }


  void RepoManager::Impl::addRepositories( const Url & url, const ProgressData::ReceiverFnc & progressrcv )
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
    filesystem::assert_dir(_options.knownReposPath);

    Pathname repofile = generateNonExistingName(_options.knownReposPath, filename);
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
      reposManip().insert(*it);

      HistoryLog(_options.rootDir).addRepository(*it);
    }

    MIL << "done" << endl;
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::Impl::removeRepository( const RepoInfo & info, const ProgressData::ReceiverFnc & progressrcv )
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
        ZYPP_THROW(RepoException( todelete, _("Can't figure out where the repo is stored.") ));
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
            ZYPP_THROW(RepoException( todelete, str::form( _("Can't delete '%s'"), todelete.filepath().c_str() )));
          }
          MIL << todelete.alias() << " successfully deleted." << endl;
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

        CombinedProgressData cSubprogrcv(progress, 20);
        CombinedProgressData mSubprogrcv(progress, 40);
        CombinedProgressData pSubprogrcv(progress, 40);
        // now delete it from cache
        if ( isCached(todelete) )
          cleanCache( todelete, cSubprogrcv);
        // now delete metadata (#301037)
        cleanMetadata( todelete, mSubprogrcv );
	cleanPackages( todelete, pSubprogrcv );
        reposManip().erase(todelete);
        MIL << todelete.alias() << " successfully deleted." << endl;
        HistoryLog(_options.rootDir).removeRepository(todelete);
        return;
      } // else filepath is empty

    }
    // should not be reached on a sucess workflow
    ZYPP_THROW(RepoNotFoundException(info));
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::Impl::modifyRepository( const std::string & alias, const RepoInfo & newinfo_r, const ProgressData::ReceiverFnc & progressrcv )
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
      ZYPP_THROW(RepoException( toedit, _("Can't figure out where the repo is stored.") ));
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

      if ( toedit.enabled() && !newinfo.enabled() )
      {
	// On the fly remove solv.idx files for bash completion if a repo gets disabled.
	const Pathname & solvidx = solv_path_for_repoinfo(_options, newinfo)/"solv.idx";
	if ( PathInfo(solvidx).isExist() )
	  filesystem::unlink( solvidx );
      }

      newinfo.setFilepath(toedit.filepath());
      reposManip().erase(toedit);
      reposManip().insert(newinfo);
      HistoryLog(_options.rootDir).modifyRepository(toedit, newinfo);
      MIL << "repo " << alias << " modified" << endl;
    }
  }

  ////////////////////////////////////////////////////////////////////////////

  RepoInfo RepoManager::Impl::getRepositoryInfo( const std::string & alias, const ProgressData::ReceiverFnc & progressrcv )
  {
    RepoConstIterator it( findAlias( alias, repos() ) );
    if ( it != repos().end() )
      return *it;
    RepoInfo info;
    info.setAlias( alias );
    ZYPP_THROW( RepoNotFoundException(info) );
  }


  RepoInfo RepoManager::Impl::getRepositoryInfo( const Url & url, const url::ViewOption & urlview, const ProgressData::ReceiverFnc & progressrcv )
  {
    for_( it, repoBegin(), repoEnd() )
    {
      for_( urlit, (*it).baseUrlsBegin(), (*it).baseUrlsEnd() )
      {
        if ( (*urlit).asString(urlview) == url.asString(urlview) )
	  return *it;
      }
    }
    RepoInfo info;
    info.setBaseUrl( url );
    ZYPP_THROW( RepoNotFoundException(info) );
  }

  ////////////////////////////////////////////////////////////////////////////
  //
  // Services
  //
  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::Impl::addService( const ServiceInfo & service )
  {
    assert_alias( service );

    // check if service already exists
    if ( hasService( service.alias() ) )
      ZYPP_THROW( ServiceAlreadyExistsException( service ) );

    // Writable ServiceInfo is needed to save the location
    // of the .service file. Finaly insert into the service list.
    ServiceInfo toSave( service );
    saveService( toSave );
    _services.insert( toSave );

    // check for credentials in Url (username:password, not ?credentials param)
    if ( toSave.url().hasCredentialsInAuthority() )
    {
      media::CredentialManager cm(
          media::CredManagerOptions(_options.rootDir) );

      //! \todo use a method calling UI callbacks to ask where to save creds?
      cm.saveInUser(media::AuthData(toSave.url()));
    }

    MIL << "added service " << toSave.alias() << endl;
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::Impl::removeService( const std::string & alias )
  {
    MIL << "Going to delete service " << alias << endl;

    const ServiceInfo & service = getService( alias );

    Pathname location = service.filepath();
    if( location.empty() )
    {
      ZYPP_THROW(ServiceException( service, _("Can't figure out where the service is stored.") ));
    }

    ServiceSet tmpSet;
    parser::ServiceFileReader( location, ServiceCollector(tmpSet) );

    // only one service definition in the file
    if ( tmpSet.size() == 1 )
    {
      if ( filesystem::unlink(location) != 0 )
      {
        // TranslatorExplanation '%s' is a filename
        ZYPP_THROW(ServiceException( service, str::form( _("Can't delete '%s'"), location.c_str() ) ));
      }
      MIL << alias << " successfully deleted." << endl;
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

      MIL << alias << " successfully deleted from file " << location <<  endl;
    }

    // now remove all repositories added by this service
    RepoCollector rcollector;
    getRepositoriesInService( alias,
			      boost::make_function_output_iterator( bind( &RepoCollector::collect, &rcollector, _1 ) ) );
    // cannot do this directly in getRepositoriesInService - would invalidate iterators
    for_(rit, rcollector.repos.begin(), rcollector.repos.end())
      removeRepository(*rit);
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::Impl::refreshServices( const RefreshServiceOptions & options_r )
  {
    // copy the set of services since refreshService
    // can eventually invalidate the iterator
    ServiceSet services( serviceBegin(), serviceEnd() );
    for_( it, services.begin(), services.end() )
    {
      if ( !it->enabled() )
        continue;

      try {
	refreshService(*it, options_r);
      }
      catch ( const repo::ServicePluginInformalException & e )
      { ;/* ignore ServicePluginInformalException */ }
    }
  }

  void RepoManager::Impl::refreshService( const std::string & alias, const RefreshServiceOptions & options_r )
  {
    ServiceInfo service( getService( alias ) );
    assert_alias( service );
    assert_url( service );
    MIL << "Going to refresh service '" << service.alias() <<  "', url: " << service.url() << ", opts: " << options_r << endl;

    if ( service.ttl() && !options_r.testFlag( RefreshService_forceRefresh ) )
    {
      // Service defines a TTL; maybe we can re-use existing data without refresh.
      Date lrf = service.lrf();
      if ( lrf )
      {
	Date now( Date::now() );
	if ( lrf <= now )
	{
	  if ( (lrf+=service.ttl()) > now ) // lrf+= !
	  {
	    MIL << "Skip: '" << service.alias() << "' metadata valid until " << lrf << endl;
	    return;
	  }
	}
	else
	  WAR << "Force: '" << service.alias() << "' metadata last refresh in the future: " << lrf << endl;
      }
    }

    // NOTE: It might be necessary to modify and rewrite the service info.
    // Either when probing the type, or when adjusting the repositories
    // enable/disable state.:
    bool serviceModified = false;

    //! \todo add callbacks for apps (start, end, repo removed, repo added, repo changed)?

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
    std::string servicesTargetDistro = _options.servicesTargetDistro;
    if ( servicesTargetDistro.empty() )
    {
      servicesTargetDistro = Target::targetDistribution( Pathname() );
    }
    DBG << "ServicesTargetDistro: " << servicesTargetDistro << endl;

    // parse it
    Date::Duration origTtl = service.ttl();	// FIXME Ugly hack: const service.ttl modified when parsing
    RepoCollector collector(servicesTargetDistro);
    // FIXME Ugly hack: ServiceRepos may throw ServicePluginInformalException
    // which is actually a notification. Using an exception for this
    // instead of signal/callback is bad. Needs to be fixed here, in refreshServices()
    // and in zypper.
    std::pair<DefaultIntegral<bool,false>, repo::ServicePluginInformalException> uglyHack;
    try {
      ServiceRepos( service, bind( &RepoCollector::collect, &collector, _1 ) );
    }
    catch ( const repo::ServicePluginInformalException & e )
    {
      /* ignore ServicePluginInformalException and throw later */
      uglyHack.first = true;
      uglyHack.second = e;
    }
    if ( service.ttl() != origTtl )	// repoindex.xml changed ttl
    {
      if ( !service.ttl() )
	service.setLrf( Date() );	// don't need lrf when zero ttl
      serviceModified = true;
    }
    ////////////////////////////////////////////////////////////////////////////
    // On the fly remember the new repo states as defined the reopoindex.xml.
    // Move into ServiceInfo later.
    ServiceInfo::RepoStates newRepoStates;

    // set service alias and base url for all collected repositories
    for_( it, collector.repos.begin(), collector.repos.end() )
    {
      // First of all: Prepend service alias:
      it->setAlias( str::form( "%s:%s", service.alias().c_str(), it->alias().c_str() ) );
      // set refrence to the parent service
      it->setService( service.alias() );

      // remember the new parsed repo state
      newRepoStates[it->alias()] = *it;

      // if the repo url was not set by the repoindex parser, set service's url
      Url url;
      if ( it->baseUrlsEmpty() )
        url = service.rawUrl();
      else
      {
        // service repo can contain only one URL now, so no need to iterate.
        url = it->rawUrl();	// raw!
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

      // save the url
      it->setBaseUrl( url );
    }

    ////////////////////////////////////////////////////////////////////////////
    // Now compare collected repos with the ones in the system...
    //
    RepoInfoList oldRepos;
    getRepositoriesInService( service.alias(), std::back_inserter( oldRepos ) );

    ////////////////////////////////////////////////////////////////////////////
    // find old repositories to remove...
    for_( oldRepo, oldRepos.begin(), oldRepos.end() )
    {
      if ( ! foundAliasIn( oldRepo->alias(), collector.repos ) )
      {
	if ( oldRepo->enabled() )
	{
	  // Currently enabled. If this was a user modification remember the state.
	  const auto & last = service.repoStates().find( oldRepo->alias() );
	  if ( last != service.repoStates().end() && ! last->second.enabled )
	  {
	    DBG << "Service removes user enabled repo " << oldRepo->alias() << endl;
	    service.addRepoToEnable( oldRepo->alias() );
	    serviceModified = true;
	  }
	  else
	    DBG << "Service removes enabled repo " << oldRepo->alias() << endl;
	}
	else
	  DBG << "Service removes disabled repo " << oldRepo->alias() << endl;

        removeRepository( *oldRepo );
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    // create missing repositories and modify exising ones if needed...
    for_( it, collector.repos.begin(), collector.repos.end() )
    {
      // User explicitly requested the repo being enabled?
      // User explicitly requested the repo being disabled?
      // And hopefully not both ;) If so, enable wins.

      TriBool toBeEnabled( indeterminate );	// indeterminate - follow the service request
      DBG << "Service request to " << (it->enabled()?"enable":"disable") << " service repo " << it->alias() << endl;

      if ( options_r.testFlag( RefreshService_restoreStatus ) )
      {
	DBG << "Opt RefreshService_restoreStatus " << it->alias() << endl;
	// this overrides any pending request!
	// Remove from enable request list.
	// NOTE: repoToDisable is handled differently.
	//       It gets cleared on each refresh.
	service.delRepoToEnable( it->alias() );
	// toBeEnabled stays indeterminate!
      }
      else
      {
	if ( service.repoToEnableFind( it->alias() ) )
	{
	  DBG << "User request to enable service repo " << it->alias() << endl;
	  toBeEnabled = true;
	  // Remove from enable request list.
	  // NOTE: repoToDisable is handled differently.
	  //       It gets cleared on each refresh.
	  service.delRepoToEnable( it->alias() );
	  serviceModified = true;
	}
	else if ( service.repoToDisableFind( it->alias() ) )
	{
	  DBG << "User request to disable service repo " << it->alias() << endl;
	  toBeEnabled = false;
	}
      }

      RepoInfoList::iterator oldRepo( findAlias( it->alias(), oldRepos ) );
      if ( oldRepo == oldRepos.end() )
      {
        // Not found in oldRepos ==> a new repo to add

	// Make sure the service repo is created with the appropriate enablement
	if ( ! indeterminate(toBeEnabled) )
	  it->setEnabled( toBeEnabled );

        DBG << "Service adds repo " << it->alias() << " " << (it->enabled()?"enabled":"disabled") << endl;
        addRepository( *it );
      }
      else
      {
        // ==> an exising repo to check
        bool oldRepoModified = false;

	if ( indeterminate(toBeEnabled) )
	{
	  // No user request: check for an old user modificaton otherwise follow service request.
	  // NOTE: Assert toBeEnabled is boolean afterwards!
	  if ( oldRepo->enabled() == it->enabled() )
	    toBeEnabled = it->enabled();	// service requests no change to the system
	  else if (options_r.testFlag( RefreshService_restoreStatus ) )
	  {
	    toBeEnabled = it->enabled();	// RefreshService_restoreStatus forced
	    DBG << "Opt RefreshService_restoreStatus " << it->alias() <<  " forces " << (toBeEnabled?"enabled":"disabled") << endl;
	  }
	  else
	  {
	    const auto & last = service.repoStates().find( oldRepo->alias() );
	    if ( last == service.repoStates().end() || last->second.enabled != it->enabled() )
	      toBeEnabled = it->enabled();	// service request has changed since last refresh -> follow
	    else
	    {
	      toBeEnabled = oldRepo->enabled();	// service request unchaned since last refresh -> keep user modification
	      DBG << "User modified service repo " << it->alias() <<  " may stay " << (toBeEnabled?"enabled":"disabled") << endl;
	    }
	  }
	}

        // changed enable?
	if ( toBeEnabled == oldRepo->enabled() )
	{
	  DBG << "Service repo " << it->alias() << " stays " <<  (oldRepo->enabled()?"enabled":"disabled") << endl;
	}
	else if ( toBeEnabled )
	{
	  DBG << "Service repo " << it->alias() << " gets enabled" << endl;
	  oldRepo->setEnabled( true );
	  oldRepoModified = true;
	}
	else
        {
	  DBG << "Service repo " << it->alias() << " gets disabled" << endl;
	  oldRepo->setEnabled( false );
	  oldRepoModified = true;
	}

	// all other attributes follow the service request:

	// changed name (raw!)
	if ( oldRepo->rawName() != it->rawName() )
	{
	  DBG << "Service repo " << it->alias() << " gets new NAME " << it->rawName() << endl;
	  oldRepo->setName( it->rawName() );
	  oldRepoModified = true;
	}

	// changed autorefresh
	if ( oldRepo->autorefresh() != it->autorefresh() )
	{
	  DBG << "Service repo " << it->alias() << " gets new AUTOREFRESH " << it->autorefresh() << endl;
	  oldRepo->setAutorefresh( it->autorefresh() );
	  oldRepoModified = true;
	}

	// changed priority?
	if ( oldRepo->priority() != it->priority() )
	{
	  DBG << "Service repo " << it->alias() << " gets new PRIORITY " << it->priority() << endl;
	  oldRepo->setPriority( it->priority() );
	  oldRepoModified = true;
	}

        // changed url?
        // service repo can contain only one URL now, so no need to iterate.
        if ( oldRepo->rawUrl() != it->rawUrl() )
        {
          DBG << "Service repo " << it->alias() << " gets new URL " << it->rawUrl() << endl;
          oldRepo->setBaseUrl( it->rawUrl() );
          oldRepoModified = true;
        }

        // changed gpg check settings?
	// ATM only plugin services can set GPG values.
	if ( service.type() == ServiceType::PLUGIN )
	{
	  TriBool ogpg[3];	// Gpg RepoGpg PkgGpg
	  TriBool ngpg[3];
	  oldRepo->getRawGpgChecks( ogpg[0], ogpg[1], ogpg[2] );
	  it->     getRawGpgChecks( ngpg[0], ngpg[1], ngpg[2] );
#define Z_CHKGPG(I,N)										\
	  if ( ! sameTriboolState( ogpg[I], ngpg[I] ) )						\
	  {											\
	    DBG << "Service repo " << it->alias() << " gets new "#N"Check " << ngpg[I] << endl;	\
	    oldRepo->set##N##Check( ngpg[I] );							\
	    oldRepoModified = true;								\
	  }
	  Z_CHKGPG( 0, Gpg );
	  Z_CHKGPG( 1, RepoGpg );
	  Z_CHKGPG( 2, PkgGpg );
#undef Z_CHKGPG
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

    // Remember original service request for next refresh
    if ( service.repoStates() != newRepoStates )
    {
      service.setRepoStates( std::move(newRepoStates) );
      serviceModified = true;
    }

    ////////////////////////////////////////////////////////////////////////////
    // save service if modified: (unless a plugin service)
    if ( service.type() != ServiceType::PLUGIN )
    {
      if ( service.ttl() )
      {
	service.setLrf( Date::now() );	// remember last refresh
	serviceModified =  true;	// or use a cookie file
      }

      if ( serviceModified )
      {
	// write out modified service file.
	modifyService( service.alias(), service );
      }
    }

    if ( uglyHack.first )
    {
      throw( uglyHack.second ); // intentionally not ZYPP_THROW
    }
  }

  ////////////////////////////////////////////////////////////////////////////

  void RepoManager::Impl::modifyService( const std::string & oldAlias, const ServiceInfo & newService )
  {
    MIL << "Going to modify service " << oldAlias << endl;

    // we need a writable copy to link it to the file where
    // it is saved if we modify it
    ServiceInfo service(newService);

    if ( service.type() == ServiceType::PLUGIN )
    {
      ZYPP_THROW(ServicePluginImmutableException( service ));
    }

    const ServiceInfo & oldService = getService(oldAlias);

    Pathname location = oldService.filepath();
    if( location.empty() )
    {
      ZYPP_THROW(ServiceException( oldService, _("Can't figure out where the service is stored.") ));
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

    _services.erase(oldAlias);
    _services.insert(service);

    // changed properties affecting also repositories
    if ( oldAlias != service.alias()			// changed alias
      || oldService.enabled() != service.enabled() )	// changed enabled status
    {
      std::vector<RepoInfo> toModify;
      getRepositoriesInService(oldAlias, std::back_inserter(toModify));
      for_( it, toModify.begin(), toModify.end() )
      {
	if ( oldService.enabled() != service.enabled() )
	{
	  if ( service.enabled() )
	  {
	    // reset to last refreshs state
	    const auto & last = service.repoStates().find( it->alias() );
	    if ( last != service.repoStates().end() )
	      it->setEnabled( last->second.enabled );
	  }
	  else
	    it->setEnabled( false );
	}

        if ( oldAlias != service.alias() )
          it->setService(service.alias());

        modifyRepository(it->alias(), *it);
      }
    }

    //! \todo refresh the service automatically if url is changed?
  }

  ////////////////////////////////////////////////////////////////////////////

  repo::ServiceType RepoManager::Impl::probeService( const Url & url ) const
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

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoManager
  //
  ///////////////////////////////////////////////////////////////////

  RepoManager::RepoManager( const RepoManagerOptions & opt )
  : _pimpl( new Impl(opt) )
  {}

  RepoManager::~RepoManager()
  {}

  bool RepoManager::repoEmpty() const
  { return _pimpl->repoEmpty(); }

  RepoManager::RepoSizeType RepoManager::repoSize() const
  { return _pimpl->repoSize(); }

  RepoManager::RepoConstIterator RepoManager::repoBegin() const
  { return _pimpl->repoBegin(); }

  RepoManager::RepoConstIterator RepoManager::repoEnd() const
  { return _pimpl->repoEnd(); }

  RepoInfo RepoManager::getRepo( const std::string & alias ) const
  { return _pimpl->getRepo( alias ); }

  bool RepoManager::hasRepo( const std::string & alias ) const
  { return _pimpl->hasRepo( alias ); }

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

  RepoStatus RepoManager::metadataStatus( const RepoInfo & info ) const
  { return _pimpl->metadataStatus( info ); }

  RepoManager::RefreshCheckStatus RepoManager::checkIfToRefreshMetadata( const RepoInfo &info, const Url &url, RawMetadataRefreshPolicy policy )
  { return _pimpl->checkIfToRefreshMetadata( info, url, policy ); }

  Pathname RepoManager::metadataPath( const RepoInfo &info ) const
  { return _pimpl->metadataPath( info ); }

  Pathname RepoManager::packagesPath( const RepoInfo &info ) const
  { return _pimpl->packagesPath( info ); }

  void RepoManager::refreshMetadata( const RepoInfo &info, RawMetadataRefreshPolicy policy, const ProgressData::ReceiverFnc & progressrcv )
  { return _pimpl->refreshMetadata( info, policy, progressrcv ); }

  void RepoManager::cleanMetadata( const RepoInfo &info, const ProgressData::ReceiverFnc & progressrcv )
  { return _pimpl->cleanMetadata( info, progressrcv ); }

  void RepoManager::cleanPackages( const RepoInfo &info, const ProgressData::ReceiverFnc & progressrcv )
  { return _pimpl->cleanPackages( info, progressrcv ); }

  RepoStatus RepoManager::cacheStatus( const RepoInfo &info ) const
  { return _pimpl->cacheStatus( info ); }

  void RepoManager::buildCache( const RepoInfo &info, CacheBuildPolicy policy, const ProgressData::ReceiverFnc & progressrcv )
  { return _pimpl->buildCache( info, policy, progressrcv ); }

  void RepoManager::cleanCache( const RepoInfo &info, const ProgressData::ReceiverFnc & progressrcv )
  { return _pimpl->cleanCache( info, progressrcv ); }

  bool RepoManager::isCached( const RepoInfo &info ) const
  { return _pimpl->isCached( info ); }

  void RepoManager::loadFromCache( const RepoInfo &info, const ProgressData::ReceiverFnc & progressrcv )
  { return _pimpl->loadFromCache( info, progressrcv ); }

  void RepoManager::cleanCacheDirGarbage( const ProgressData::ReceiverFnc & progressrcv )
  { return _pimpl->cleanCacheDirGarbage( progressrcv ); }

  repo::RepoType RepoManager::probe( const Url & url, const Pathname & path ) const
  { return _pimpl->probe( url, path ); }

  repo::RepoType RepoManager::probe( const Url & url ) const
  { return _pimpl->probe( url ); }

  void RepoManager::addRepository( const RepoInfo &info, const ProgressData::ReceiverFnc & progressrcv )
  { return _pimpl->addRepository( info, progressrcv ); }

  void RepoManager::addRepositories( const Url &url, const ProgressData::ReceiverFnc & progressrcv )
  { return _pimpl->addRepositories( url, progressrcv ); }

  void RepoManager::removeRepository( const RepoInfo & info, const ProgressData::ReceiverFnc & progressrcv )
  { return _pimpl->removeRepository( info, progressrcv ); }

  void RepoManager::modifyRepository( const std::string &alias, const RepoInfo & newinfo, const ProgressData::ReceiverFnc & progressrcv )
  { return _pimpl->modifyRepository( alias, newinfo, progressrcv ); }

  RepoInfo RepoManager::getRepositoryInfo( const std::string &alias, const ProgressData::ReceiverFnc & progressrcv )
  { return _pimpl->getRepositoryInfo( alias, progressrcv ); }

  RepoInfo RepoManager::getRepositoryInfo( const Url & url, const url::ViewOption & urlview, const ProgressData::ReceiverFnc & progressrcv )
  { return _pimpl->getRepositoryInfo( url, urlview, progressrcv ); }

  bool RepoManager::serviceEmpty() const
  { return _pimpl->serviceEmpty(); }

  RepoManager::ServiceSizeType RepoManager::serviceSize() const
  { return _pimpl->serviceSize(); }

  RepoManager::ServiceConstIterator RepoManager::serviceBegin() const
  { return _pimpl->serviceBegin(); }

  RepoManager::ServiceConstIterator RepoManager::serviceEnd() const
  { return _pimpl->serviceEnd(); }

  ServiceInfo RepoManager::getService( const std::string & alias ) const
  { return _pimpl->getService( alias ); }

  bool RepoManager::hasService( const std::string & alias ) const
  { return _pimpl->hasService( alias ); }

  repo::ServiceType RepoManager::probeService( const Url &url ) const
  { return _pimpl->probeService( url ); }

  void RepoManager::addService( const std::string & alias, const Url& url )
  { return _pimpl->addService( alias, url ); }

  void RepoManager::addService( const ServiceInfo & service )
  { return _pimpl->addService( service ); }

  void RepoManager::removeService( const std::string & alias )
  { return _pimpl->removeService( alias ); }

  void RepoManager::removeService( const ServiceInfo & service )
  { return _pimpl->removeService( service ); }

  void RepoManager::refreshServices( const RefreshServiceOptions & options_r )
  { return _pimpl->refreshServices( options_r ); }

  void RepoManager::refreshService( const std::string & alias, const RefreshServiceOptions & options_r )
  { return _pimpl->refreshService( alias, options_r ); }

  void RepoManager::refreshService( const ServiceInfo & service, const RefreshServiceOptions & options_r )
  { return _pimpl->refreshService( service, options_r ); }

  void RepoManager::modifyService( const std::string & oldAlias, const ServiceInfo & service )
  { return _pimpl->modifyService( oldAlias, service ); }

  ////////////////////////////////////////////////////////////////////////////

  std::ostream & operator<<( std::ostream & str, const RepoManager & obj )
  { return str << *obj._pimpl; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
