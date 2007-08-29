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

#include <iostream>
#include <fstream>
#include <list>
#include <algorithm>
#include "zypp/base/InputStream.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Function.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"

#include "zypp/repo/RepoException.h"
#include "zypp/RepoManager.h"

#include "zypp/cache/CacheStore.h"
#include "zypp/repo/cached/RepoImpl.h"
#include "zypp/media/MediaManager.h"
#include "zypp/MediaSetAccess.h"

#include "zypp/parser/RepoFileReader.h"
#include "zypp/repo/yum/Downloader.h"
#include "zypp/parser/yum/RepoParser.h"
#include "zypp/parser/plaindir/RepoParser.h"
#include "zypp/repo/susetags/Downloader.h"
#include "zypp/parser/susetags/RepoParser.h"

#include "zypp/ZYppCallbacks.h"

using namespace std;
using namespace zypp;
using namespace zypp::repo;
using namespace zypp::filesystem;

using namespace zypp::repo;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoManagerOptions
  //
  ///////////////////////////////////////////////////////////////////

  RepoManagerOptions::RepoManagerOptions()
  {
    repoCachePath    = ZConfig::instance().repoCachePath();
    repoRawCachePath = ZConfig::instance().repoMetadataPath();
    knownReposPath   = ZConfig::instance().knownReposPath();
  }

  ////////////////////////////////////////////////////////////////////////////

  /**
    * \short Simple callback to collect the results
    *
    * Classes like RepoFileParser call the callback
    * once per each repo in a file.
    *
    * Passing this functor as callback, you can collect
    * all resuls at the end, without dealing with async
    * code.
    */
    struct RepoCollector
    {
      RepoCollector()
      {
        MIL << endl;
      }

      ~RepoCollector()
      {
        MIL << endl;
      }

      bool collect( const RepoInfo &repo )
      {
        //MIL << "here in collector: " << repo.alias() << endl;
        repos.push_back(repo);
        //MIL << "added: " << repo.alias() << endl;
        return true;
      }

      RepoInfoList repos;
    };

  ////////////////////////////////////////////////////////////////////////////

   /**
    * \short Internal version of clean cache
    *
    * Takes an extra CacheStore reference, so we avoid internally
    * having 2 CacheStores writing to the same database.
    */
  static void cleanCacheInternal( cache::CacheStore &store,
                                  const RepoInfo &info,
                                  const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() )
  {
    ProgressData progress;
    callback::SendReport<ProgressReport> report;
    progress.sendTo( ProgressReportAdaptor( progressrcv, report ) );
    progress.name(str::form(_("Cleaning repository '%s' cache"), info.name().c_str()));

    if ( !store.isCached(info.alias()) )
      return;
   
    MIL << info.alias() << " cleaning cache..." << endl;
    data::RecordId id = store.lookupRepository(info.alias());
    
    CombinedProgressData subprogrcv(progress);
    
    store.cleanRepository(id, subprogrcv);
  }
  
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

  /**
   * \short List of RepoInfo's from a directory
   *
   * Goes trough every file in a directory and adds all
   * RepoInfo's contained in that file.
   *
   * \param dir pathname of the directory to read.
   */
  static std::list<RepoInfo> repositories_in_dir( const Pathname &dir )
  {
    MIL << "directory " << dir << endl;
    list<RepoInfo> repos;
    list<Pathname> entries;
    if ( filesystem::readdir( entries, Pathname(dir), false ) != 0 )
      ZYPP_THROW(Exception("failed to read directory"));

    for ( list<Pathname>::const_iterator it = entries.begin(); it != entries.end(); ++it )
    {
      list<RepoInfo> tmp = repositories_in_file( *it );
      repos.insert( repos.end(), tmp.begin(), tmp.end() );

      //std::copy( collector.repos.begin(), collector.repos.end(), std::back_inserter(repos));
      //MIL << "ok" << endl;
    }
    return repos;
  }

  ////////////////////////////////////////////////////////////////////////////

  static void assert_alias( const RepoInfo &info )
  {
    if (info.alias().empty())
        ZYPP_THROW(RepoNoAliasException());
  }

  ////////////////////////////////////////////////////////////////////////////

  static void assert_urls( const RepoInfo &info )
  {
    if (info.baseUrlsEmpty())
        ZYPP_THROW(RepoNoUrlException());
  }

  ////////////////////////////////////////////////////////////////////////////

  /**
   * \short Calculates the raw cache path for a repository
   */
  static Pathname rawcache_path_for_repoinfo( const RepoManagerOptions &opt, const RepoInfo &info )
  {
    assert_alias(info);
    return opt.repoRawCachePath + info.alias();
  }

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

    }

    Impl()
    {

    }

    RepoManagerOptions options;

  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl );
      return _nullimpl;
    }
    
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

  std::list<RepoInfo> RepoManager::knownRepositories() const
  {
    MIL << endl;

    if ( PathInfo(_pimpl->options.knownReposPath).isExist() )
    {
      RepoInfoList repos = repositories_in_dir(_pimpl->options.knownReposPath);
      for ( RepoInfoList::iterator it = repos.begin();
            it != repos.end();
            ++it )
      {
        // set the metadata path for the repo
        Pathname metadata_path = rawcache_path_for_repoinfo(_pimpl->options, (*it));
        (*it).setMetadataPath(metadata_path);
      }
      return repos;
    }
    else
      return std::list<RepoInfo>();

    MIL << endl;
  }

  ////////////////////////////////////////////////////////////////////////////

  RepoStatus RepoManager::metadataStatus( const RepoInfo &info ) const
  {
    Pathname rawpath = rawcache_path_for_repoinfo( _pimpl->options, info );
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
        status = (RepoStatus( rawpath + "/media.1/media") && RepoStatus( rawpath + "/content") );
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
    Pathname rawpath = rawcache_path_for_repoinfo( _pimpl->options, info );

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

  bool RepoManager::checkIfToRefreshMetadata( const RepoInfo &info,
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

      Pathname rawpath = rawcache_path_for_repoinfo( _pimpl->options, info );
      filesystem::assert_dir(rawpath);
      oldstatus = metadataStatus(info);

      // now we've got the old (cached) status, we can decide repo.refresh.delay
      if (policy != RefreshForced)
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
          return false;
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
          downloader_ptr.reset(new yum::Downloader(info.path()));
        else
          downloader_ptr.reset( new susetags::Downloader(info.path()));

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

        return refresh;
      }
      else if ( repokind.toEnum() == RepoType::RPMPLAINDIR_e )
      {
        RepoStatus newstatus = parser::plaindir::dirStatus(url.getPathName());
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

        return refresh;
      }
      else
      {
        ZYPP_THROW(RepoUnknownTypeException());
      }
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      ERR << "refresh check failed for " << url << endl;
      ZYPP_RETHROW(e);
    }
    
    return true; // default
  }

  void RepoManager::refreshMetadata( const RepoInfo &info,
                                     RawMetadataRefreshPolicy policy,
                                     const ProgressData::ReceiverFnc & progress )
  {
    assert_alias(info);
    assert_urls(info);

    // try urls one by one
    for ( RepoInfo::urls_const_iterator it = info.baseUrlsBegin(); it != info.baseUrlsEnd(); ++it )
    {
      try
      {
        Url url(*it);

        // check whether to refresh metadata
        // if the check fails for this url, it throws, so another url will be checked
        if (!checkIfToRefreshMetadata(info, url, policy))
          return;

        MIL << "Going to refresh metadata from " << url << endl;

        repo::RepoType repokind = info.type();

        // if the type is unknown, try probing.
        switch ( repokind.toEnum() )
        {
          case RepoType::NONE_e:
            // unknown, probe it
            repokind = probe(*it);
          break;
          default:
          break;
        }

        Pathname rawpath = rawcache_path_for_repoinfo( _pimpl->options, info );
        filesystem::assert_dir(rawpath);

        // create temp dir as sibling of rawpath
        filesystem::TmpDir tmpdir( filesystem::TmpDir::makeSibling( rawpath ) );

        if ( ( repokind.toEnum() == RepoType::RPMMD_e ) ||
             ( repokind.toEnum() == RepoType::YAST2_e ) )
        {
          MediaSetAccess media(url);
          shared_ptr<repo::Downloader> downloader_ptr;

          if ( repokind.toEnum() == RepoType::RPMMD_e )
            downloader_ptr.reset(new yum::Downloader(info.path()));
          else
            downloader_ptr.reset( new susetags::Downloader(info.path()));

          /**
           * Given a downloader, sets the other repos raw metadata
           * path as cache paths for the fetcher, so if another
           * repo has the same file, it will not download it
           * but copy it from the other repository
           */
          std::list<RepoInfo> repos = knownRepositories();
          for ( std::list<RepoInfo>::const_iterator it = repos.begin();
                it != repos.end();
                ++it )
          {
            downloader_ptr->addCachePath(rawcache_path_for_repoinfo( _pimpl->options, *it ));
          }

          downloader_ptr->download( media, tmpdir.path());
        }
        else if ( repokind.toEnum() == RepoType::RPMPLAINDIR_e )
        {
          RepoStatus newstatus = parser::plaindir::dirStatus(url.getPathName());

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
        TmpDir oldmetadata( TmpDir::makeSibling( rawpath ) );
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
      }
    } // for every url
    ERR << "No more urls..." << endl;
    ZYPP_THROW(RepoException(_("Valid metadata not found at specified URL(s)")));
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

  void RepoManager::buildCache( const RepoInfo &info,
                                CacheBuildPolicy policy,
                                const ProgressData::ReceiverFnc & progressrcv )
  {
    assert_alias(info);
    Pathname rawpath = rawcache_path_for_repoinfo(_pimpl->options, info);

    cache::CacheStore store(_pimpl->options.repoCachePath);

    RepoStatus raw_metadata_status = metadataStatus(info);
    if ( raw_metadata_status.empty() )
    {
      ZYPP_THROW(RepoMetadataException(info));
    }

    bool needs_cleaning = false;
    if ( store.isCached( info.alias() ) )
    {
      MIL << info.alias() << " is already cached." << endl;
      data::RecordId id = store.lookupRepository(info.alias());
      RepoStatus cache_status = store.repositoryStatus(id);

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
      cleanCacheInternal( store, info);

    MIL << info.alias() << " building cache..." << endl;
    data::RecordId id = store.lookupOrAppendRepository(info.alias());
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

    
    switch ( repokind.toEnum() )
    {
      case RepoType::RPMMD_e :
      {
        CombinedProgressData subprogrcv( progress, 100);
        parser::yum::RepoParser parser(id, store, parser::yum::RepoParserOpts(), subprogrcv);
        parser.parse(rawpath);
          // no error
      }
      break;
      case RepoType::YAST2_e :
      {
        CombinedProgressData subprogrcv( progress, 100);
        parser::susetags::RepoParser parser(id, store, subprogrcv);
        parser.parse(rawpath);
        // no error
      }
      break;
      case RepoType::RPMPLAINDIR_e :
      {
        CombinedProgressData subprogrcv( progress, 100);
        InputStream is(rawpath + "cookie");
        string buffer;
        getline( is.stream(), buffer);
        Url url(buffer);
        parser::plaindir::RepoParser parser(id, store, subprogrcv);
        parser.parse(url.getPathName());
      }
      break;
      default:
        ZYPP_THROW(RepoUnknownTypeException());
    }

    // update timestamp and checksum
    store.updateRepositoryStatus(id, raw_metadata_status);

    MIL << "Commit cache.." << endl;
    store.commit();
    //progress.toMax();
  }

  ////////////////////////////////////////////////////////////////////////////

  repo::RepoType RepoManager::probe( const Url &url ) const
  {
    if ( url.getScheme() == "dir" && ! PathInfo( url.getPathName() ).isDir() )
    {
      // Handle non existing local directory in advance, as
      // MediaSetAccess does not support it.
      return repo::RepoType::NONE;
    }

    try
    {
      MediaSetAccess access(url);
      if ( access.doesFileExist("/repodata/repomd.xml") )
        return repo::RepoType::RPMMD;
      if ( access.doesFileExist("/content") )
        return repo::RepoType::YAST2;
  
      // if it is a local url of type dir
      if ( (! media::MediaManager::downloads(url)) && ( url.getScheme() == "dir" ) )
      {
        Pathname path = Pathname(url.getPathName());
        if ( PathInfo(path).isDir() )
        {
          // allow empty dirs for now
          return repo::RepoType::RPMPLAINDIR;
        }
      }
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

    return repo::RepoType::NONE;
  }
    
  ////////////////////////////////////////////////////////////////////////////
  
  void RepoManager::cleanCache( const RepoInfo &info,
                                const ProgressData::ReceiverFnc & progressrcv )
  {
    cache::CacheStore store(_pimpl->options.repoCachePath);
    cleanCacheInternal( store, info, progressrcv );
    store.commit();
  }

  ////////////////////////////////////////////////////////////////////////////

  bool RepoManager::isCached( const RepoInfo &info ) const
  {
    cache::CacheStore store(_pimpl->options.repoCachePath);
    return store.isCached(info.alias());
  }

  RepoStatus RepoManager::cacheStatus( const RepoInfo &info ) const
  {
    cache::CacheStore store(_pimpl->options.repoCachePath);
    data::RecordId id = store.lookupRepository(info.alias());
    RepoStatus cache_status = store.repositoryStatus(id);
    return cache_status;
  }

  Repository RepoManager::createFromCache( const RepoInfo &info,
                                           const ProgressData::ReceiverFnc & progressrcv )
  {
    callback::SendReport<ProgressReport> report;
    ProgressData progress;
    progress.sendTo(ProgressReportAdaptor( progressrcv, report ));
    //progress.sendTo( progressrcv );
    progress.name(str::form(_("Reading repository '%s' cache"), info.name().c_str()));
    
    cache::CacheStore store(_pimpl->options.repoCachePath);

    if ( ! store.isCached( info.alias() ) )
      ZYPP_THROW(RepoNotCachedException());

    MIL << "Repository " << info.alias() << " is cached" << endl;

    data::RecordId id = store.lookupRepository(info.alias());
    
    CombinedProgressData subprogrcv(progress);
    
    repo::cached::RepoOptions opts( info, _pimpl->options.repoCachePath, id );
    opts.readingResolvablesProgress = subprogrcv;
    repo::cached::RepoImpl::Ptr repoimpl =
        new repo::cached::RepoImpl( opts );

    repoimpl->resolvables();
    // read the resolvables from cache
    return Repository(repoimpl);
  }

  ////////////////////////////////////////////////////////////////////////////

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
  static Pathname generate_non_existing_name( const Pathname &dir,
                                              const std::string &basefilename )
  {
    string final_filename = basefilename;
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
  static std::string generate_filename( const RepoInfo &info )
  {
    std::string fnd="/";
    std::string rep="_";
    std::string filename = info.alias();
    // replace slashes with underscores
    size_t pos = filename.find(fnd);
    while(pos!=string::npos)
    {
      filename.replace(pos,fnd.length(),rep);
      pos = filename.find(fnd,pos+rep.length());
    }
    filename = Pathname(filename).extend(".repo").asString();
    MIL << "generating filename for repo [" << info.alias() << "] : '" << filename << "'" << endl;
    return filename;
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

    std::list<RepoInfo> repos = knownRepositories();
    for ( std::list<RepoInfo>::const_iterator it = repos.begin();
          it != repos.end();
          ++it )
    {
      if ( info.alias() == (*it).alias() )
        ZYPP_THROW(RepoAlreadyExistsException(info.alias()));
    }

    RepoInfo tosave = info;
    
    // check the first url for now
    if ( ZConfig::instance().repo_add_probe() || ( tosave.type() == RepoType::NONE ) )
    {
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

    Pathname repofile = generate_non_existing_name(_pimpl->options.knownReposPath,
                                                    generate_filename(tosave));
    // now we have a filename that does not exists
    MIL << "Saving repo in " << repofile << endl;

    std::ofstream file(repofile.c_str());
    if (!file) {
      ZYPP_THROW (Exception( "Can't open " + repofile.asString() ) );
    }

    tosave.dumpRepoOn(file);
    progress.toMax();
    MIL << "done" << endl;
  }

  void RepoManager::addRepositories( const Url &url,
                                     const ProgressData::ReceiverFnc & progressrcv )
  {
    std::list<RepoInfo> knownrepos = knownRepositories();
    std::list<RepoInfo> repos = readRepoFile(url);
    for ( std::list<RepoInfo>::const_iterator it = repos.begin();
          it != repos.end();
          ++it )
    {
      // look if the alias is in the known repos.
      for ( std::list<RepoInfo>::const_iterator kit = knownrepos.begin();
          kit != knownrepos.end();
          ++kit )
      {
        if ( (*it).alias() == (*kit).alias() )
        {
          ERR << "To be added repo " << (*it).alias() << " conflicts with existing repo " << (*kit).alias() << endl;
          ZYPP_THROW(RepoAlreadyExistsException((*it).alias()));
        }
      }
    }

    string filename = Pathname(url.getPathName()).basename();

    if ( filename == Pathname() )
      ZYPP_THROW(RepoException("Invalid repo file name at " + url.asString() ));

    // assert the directory exists
    filesystem::assert_dir(_pimpl->options.knownReposPath);

    Pathname repofile = generate_non_existing_name(_pimpl->options.knownReposPath, filename);
    // now we have a filename that does not exists
    MIL << "Saving " << repos.size() << " repo" << ( repos.size() ? "s" : "" ) << " in " << repofile << endl;

    std::ofstream file(repofile.c_str());
    if (!file) {
      ZYPP_THROW (Exception( "Can't open " + repofile.asString() ) );
    }

    for ( std::list<RepoInfo>::const_iterator it = repos.begin();
          it != repos.end();
          ++it )
    {
      MIL << "Saving " << (*it).alias() << endl;
      (*it).dumpRepoOn(file);
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

    std::list<RepoInfo> repos = knownRepositories();
    for ( std::list<RepoInfo>::const_iterator it = repos.begin();
          it != repos.end();
          ++it )
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
              (*fit).dumpRepoOn(file);
          }
        }

        CombinedProgressData subprogrcv(progress, 70);
        CombinedProgressData cleansubprogrcv(progress, 30);
        // now delete it from cache
        cleanCache( todelete, subprogrcv);
        // now delete metadata (#301037)
        cleanMetadata( todelete, cleansubprogrcv);
        MIL << todelete.alias() << " sucessfully deleted." << endl;
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
            (*fit).dumpRepoOn(file);
          else
            newinfo.dumpRepoOn(file);
      }
    }
  }

  ////////////////////////////////////////////////////////////////////////////

  RepoInfo RepoManager::getRepositoryInfo( const std::string &alias,
                                           const ProgressData::ReceiverFnc & progressrcv )
  {
    std::list<RepoInfo> repos = knownRepositories();
    for ( std::list<RepoInfo>::const_iterator it = repos.begin();
          it != repos.end();
          ++it )
    {
      if ( (*it).alias() == alias )
        return *it;
    }
    RepoInfo info;
    info.setAlias(info.alias());
    ZYPP_THROW(RepoNotFoundException(info));
  }

  ////////////////////////////////////////////////////////////////////////////

  RepoInfo RepoManager::getRepositoryInfo( const Url & url,
                                           const url::ViewOption & urlview,
                                           const ProgressData::ReceiverFnc & progressrcv )
  {
    std::list<RepoInfo> repos = knownRepositories();
    for ( std::list<RepoInfo>::const_iterator it = repos.begin();
          it != repos.end();
          ++it )
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
    info.setAlias(info.alias());
    info.setBaseUrl(url);
    ZYPP_THROW(RepoNotFoundException(info));
  }

  ////////////////////////////////////////////////////////////////////////////

  std::ostream & operator<<( std::ostream & str, const RepoManager & obj )
  {
    return str << *obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
