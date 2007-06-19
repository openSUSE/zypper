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
#include "zypp/base/Function.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"

#include "zypp/repo/RepoException.h"
#include "zypp/RepoManager.h"

#include "zypp/cache/CacheStore.h"
#include "zypp/repo/cached/RepoImpl.h"
#include "zypp/MediaSetAccess.h"

#include "zypp/parser/RepoFileReader.h"
#include "zypp/repo/yum/Downloader.h"
#include "zypp/parser/yum/RepoParser.h"

#include "zypp/repo/susetags/Downloader.h"
#include "zypp/parser/susetags/RepoParser.h"

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
    ZConfig globalConfig;
    repoCachePath = globalConfig.defaultRepoCachePath();
    repoRawCachePath = globalConfig.defaultRepoRawCachePath();
    knownReposPath = globalConfig.defaultKnownReposPath();
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
    if (info.baseUrls().empty())
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
    return repositories_in_dir(_pimpl->options.knownReposPath);
    MIL << endl;
  }

  ////////////////////////////////////////////////////////////////////////////
  
  RepoStatus RepoManager::rawMetadataStatus( const RepoInfo &info )
  {
    Pathname rawpath = rawcache_path_for_repoinfo( _pimpl->options, info );
    RepoType repokind = info.type();
    RepoStatus status;
    switch ( repokind.toEnum() )
    {
      case RepoType::NONE_e:
      // unknown, probe the local metadata
        repokind = probe(Url(rawpath.asString()));
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
        status = RepoStatus( rawpath + "/content");
      }
      break;
      default:
        ZYPP_THROW(RepoUnknownTypeException());
    }
    return status;
  }
    
  
  void RepoManager::refreshMetadata( const RepoInfo &info,
                                     RawMetadataRefreshPolicy policy,
                                     const ProgressData::ReceiverFnc & progress )
  {
    assert_alias(info);
    assert_urls(info);
    
    RepoStatus oldstatus;
    RepoStatus newstatus;
    // try urls one by one
    for ( RepoInfo::urls_const_iterator it = info.baseUrlsBegin(); it != info.baseUrlsEnd(); ++it )
    {
      try
      {
        Url url(*it);
        filesystem::TmpDir tmpdir;
        
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
        oldstatus = rawMetadataStatus(info);
        
        switch ( repokind.toEnum() )
        {
          case RepoType::RPMMD_e :
          {
            yum::Downloader downloader( url, "/" );
            
            RepoStatus newstatus = downloader.status();
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
              refresh = true;
            }
  
            if ( refresh )
              downloader.download(tmpdir.path());
            else
              return;
            // no error
          }
          break;
          case RepoType::YAST2_e :
          {
            susetags::Downloader downloader( url, "/" );
            
            RepoStatus newstatus = downloader.status();
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
              refresh = true;
            }
  
            if ( refresh )
              downloader.download(tmpdir.path());
            else
              return;
            // no error
          }
          break;
          default:
            ZYPP_THROW(RepoUnknownTypeException());
        }
        
        // ok we have the metadata, now exchange
        // the contents
        TmpDir oldmetadata;
        filesystem::assert_dir(rawpath);
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
    ZYPP_THROW(RepoException("Cant refresh metadata"));
  }
  
  ////////////////////////////////////////////////////////////////////////////
  
  void RepoManager::cleanMetadata( const RepoInfo &info,
                                   const ProgressData::ReceiverFnc & progress )
  {
    filesystem::recursive_rmdir(rawcache_path_for_repoinfo(_pimpl->options, info));
  }
  
  ////////////////////////////////////////////////////////////////////////////
  
  void RepoManager::buildCache( const RepoInfo &info,
                                CacheBuildPolicy policy,
                                const ProgressData::ReceiverFnc & progressrcv )
  {
    ProgressData progress;
    progress.sendTo(progressrcv);
    progress.toMin();
    assert_alias(info);
    Pathname rawpath = rawcache_path_for_repoinfo(_pimpl->options, info);
    
    cache::CacheStore store(_pimpl->options.repoCachePath);
    
    RepoStatus raw_metadata_status = rawMetadataStatus(info);
    if ( store.isCached( info.alias() ) )
    {
      MIL << info.alias() << " is already cached." << endl;
      data::RecordId id = store.lookupRepository(info.alias());
      RepoStatus cache_status = store.repositoryStatus(id);

      if ( cache_status.checksum() == raw_metadata_status.checksum() )
      {
        MIL << info.alias() << " cache is up to date with metadata." << endl;
        if ( policy == BuildIfNeeded ) {
          progress.toMax();
          return;
        }
        else {
          MIL << "Build cache is forced" << endl;
        }
      }
      store.cleanRepository(id);
    }
    
    data::RecordId id = store.lookupOrAppendRepository(info.alias());
    // do we have type?
    repo::RepoType repokind = info.type();
      
      // if the type is unknown, try probing.
      switch ( repokind.toEnum() )
      {
        case RepoType::NONE_e:
          // unknown, probe the local metadata
          repokind = probe(Url(rawpath.asString()));
        break;
        default:
        break;
      }
      
      switch ( repokind.toEnum() )
      {
        case RepoType::RPMMD_e :
        {
          parser::yum::RepoParser parser(id, store);
          parser.parse(rawpath);
           // no error
        }
        break;
        case RepoType::YAST2_e :
        {
          parser::susetags::RepoParser parser(id, store);
          parser.parse(rawpath);
          // no error
        }
        break;
        default:
          ZYPP_THROW(RepoUnknownTypeException());
      }
      
      // update timestamp and checksum
      store.updateRepositoryStatus(id, raw_metadata_status);
      
      MIL << "Commit cache.." << endl;
      store.commit();
      progress.toMax();
  }
  
  ////////////////////////////////////////////////////////////////////////////
  
  repo::RepoType RepoManager::probe( const Url &url )
  {
    MediaSetAccess access(url);
    if ( access.doesFileExist("/repodata/repomd.xml") )
      return repo::RepoType::RPMMD;
    if ( access.doesFileExist("/content") )
      return repo::RepoType::YAST2;
    
    return repo::RepoType("UNKNOWN");
  }
  
  ////////////////////////////////////////////////////////////////////////////
  
  void RepoManager::cleanCache( const RepoInfo &info,
                                const ProgressData::ReceiverFnc & progressrcv )
  {
    ProgressData progress;
    progress.sendTo(progressrcv);

    cache::CacheStore store(_pimpl->options.repoCachePath);

    data::RecordId id = store.lookupRepository(info.alias());
    store.cleanRepository(id);
    store.commit();
  }
  
  ////////////////////////////////////////////////////////////////////////////
  
  bool RepoManager::isCached( const RepoInfo &info ) const
  {
    cache::CacheStore store(_pimpl->options.repoCachePath);
    return store.isCached(info.alias());
  }
  
  Repository RepoManager::createFromCache( const RepoInfo &info,
                                           const ProgressData::ReceiverFnc & progress )
  {
    cache::CacheStore store(_pimpl->options.repoCachePath);
    
    if ( ! store.isCached( info.alias() ) )
      ZYPP_THROW(RepoNotCachedException());
    
    MIL << "Repository " << info.alias() << " is cached" << endl;
    
    data::RecordId id = store.lookupRepository(info.alias());
    repo::cached::RepoImpl::Ptr repoimpl =
        new repo::cached::RepoImpl( info, _pimpl->options.repoCachePath, id );
    // read the resolvables from cache
    repoimpl->createResolvables();
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

  void RepoManager::addRepository( const RepoInfo &info,
                                   const ProgressData::ReceiverFnc & progressrcv )
  {
    assert_alias(info);
    
    std::list<RepoInfo> repos = knownRepositories();
    for ( std::list<RepoInfo>::const_iterator it = repos.begin();
          it != repos.end();
          ++it )
    {
      if ( info.alias() == (*it).alias() )
        ZYPP_THROW(RepoAlreadyExistsException(info.alias()));
    }
    
    Pathname repofile = generate_non_existing_name(_pimpl->options.knownReposPath,
                                                    Pathname(info.alias()).extend(".repo").asString());
    // now we have a filename that does not exists
    MIL << "Saving repo in " << repofile << endl;
    
    std::ofstream file(repofile.c_str());
    if (!file) {
      ZYPP_THROW (Exception( "Can't open " + repofile.asString() ) );
    }
    
    info.dumpRepoOn(file);
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
          return;
        }
        else
        {
          // there are more repos in the same file
          // write them back except the deleted one.
          //TmpFile tmp;
          //std::ofstream file(tmp.path().c_str());
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
          
          
          
          return;
        }
      } // else filepath is empty
      
    }
    // should not be reached on a sucess workflow
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
