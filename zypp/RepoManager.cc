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
  
  /**
    * \short Simple callback to collect the results
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
   * \short List of RepoInfo's from a directory
   *
   * Goes trough every file in a directory and adds all
   * RepoInfo's contained in that file.
   *
   * \param file pathname of the file to read.
   */
  static std::list<RepoInfo> repositories_in_path( const Pathname &dir )
  {
    MIL << " " << dir << endl;
    RepoCollector collector;
    std::list<RepoInfo> repos;
    list<Pathname> entries;
    if ( filesystem::readdir( entries, Pathname(dir), false ) != 0 )
      ZYPP_THROW(Exception("failed to read directory"));
    
    for ( list<Pathname>::const_iterator it = entries.begin(); it != entries.end(); ++it )
    {
      Pathname file = *it;
      parser::RepoFileReader parser( file, bind( &RepoCollector::collect, &collector, _1 ) );

      //std::copy( collector.repos.begin(), collector.repos.end(), std::back_inserter(repos));
      //MIL << "ok" << endl;
    }
    return collector.repos;
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
    return repositories_in_path("/etc/zypp/repos.d");
    MIL << endl;
  }
  
  ////////////////////////////////////////////////////////////////////////////
  
  void RepoManager::refreshMetadata( const RepoInfo &info )
  {
    assert_alias(info);
    assert_urls(info);
    
    // try urls one by one
    for ( RepoInfo::urls_const_iterator it = info.baseUrlsBegin(); it != info.baseUrlsEnd(); ++it )
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
      
      switch ( repokind.toEnum() )
      {
        case RepoType::RPMMD_e :
        {
          yum::Downloader downloader( url, "/" );
          downloader.download(tmpdir.path());
           // no error
        }
        break;
        case RepoType::YAST2_e :
        {
          susetags::Downloader downloader( url, "/" );
          downloader.download(tmpdir.path());
          // no error
        }
        break;
        default:
          ZYPP_THROW(RepoUnknownTypeException());
      }
      
      // ok we have the metadata, now exchange
      // the contents
      Pathname rawpath = rawcache_path_for_repoinfo(_pimpl->options, info);
      TmpDir oldmetadata;
      filesystem::assert_dir(rawpath);
      filesystem::rename( rawpath, oldmetadata.path() );
      // move the just downloaded there
      filesystem::rename( tmpdir.path(), rawpath );
      
      // we are done.
    }
  }
  
  ////////////////////////////////////////////////////////////////////////////
  
  void RepoManager::cleanMetadata( const RepoInfo &info )
  {
    filesystem::recursive_rmdir(rawcache_path_for_repoinfo(_pimpl->options, info));
  }
  
  ////////////////////////////////////////////////////////////////////////////
  
  void RepoManager::buildCache( const RepoInfo &info )
  {
    assert_alias(info);
    Pathname rawpath = rawcache_path_for_repoinfo(_pimpl->options, info);
    
    cache::CacheStore store(_pimpl->options.repoCachePath);
    
    if ( store.isCached( info.alias() ) )
    {
      MIL << info.alias() << " is already cached, cleaning..." << endl;
      data::RecordId id = store.lookupRepository(info.alias());
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
      
      MIL << "Commit cache.." << endl;
      store.commit();
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
  
  void RepoManager::cleanCache( const RepoInfo &info )
  {
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
  
  Repository RepoManager::createFromCache( const RepoInfo &info )
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
  
  void RepoManager::addRepository( const RepoInfo &info )
  {
  
  }
  
  ////////////////////////////////////////////////////////////////////////////
  
  std::ostream & operator<<( std::ostream & str, const RepoManager & obj )
  {
    return str << *obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
