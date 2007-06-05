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
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"
#include "zypp/parser/IniDict.h"

#include "zypp2/repo/RepoException.h"
#include "zypp2/RepoManager.h"

#include "zypp2/cache/CacheStore.h"
#include "zypp2/repo/cached/RepoImpl.h"
#include "zypp/MediaSetAccess.h"

#include "zypp/source/yum/YUMDownloader.h"
#include "zypp/parser/yum/RepoParser.h"

#include "zypp/source/susetags/SUSETagsDownloader.h"
#include "zypp2/parser/susetags/RepoParser.h"

using namespace std;
using namespace zypp;
using namespace zypp::repo;
using namespace zypp::filesystem;
using parser::IniDict;

using zypp::source::yum::YUMDownloader;
using zypp::source::susetags::SUSETagsDownloader;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  RepoManagerOptions::RepoManagerOptions()
  {
    ZConfig globalConfig;
    repoCachePath = globalConfig.defaultRepoCachePath();
    repoRawCachePath = globalConfig.defaultRepoRawCachePath();
    knownReposPath = globalConfig.defaultKnownReposPath();
  }
    
  /**
   * \short List of RepoInfo's from a file.
   * \param file pathname of the file to read.
   */
  static std::list<RepoInfo> repositories_in_file( const Pathname &file )
  {
    InputStream is(file);
    IniDict dict(is);
    std::list<RepoInfo> repos;
    
    for ( IniDict::section_const_iterator its = dict.sectionsBegin();
          its != dict.sectionsEnd();
          ++its )
    {
      MIL << (*its) << endl;
      
      RepoInfo info;
      info.setAlias(*its);
                    
      for ( IniDict::entry_const_iterator it = dict.entriesBegin(*its);
            it != dict.entriesEnd(*its);
            ++it )
      {
        
        //MIL << (*it).first << endl;
        if (it->first == "name" )
          info.setName(it-> second);
        else if ( it->first == "enabled" )
          info.setEnabled( it->second == "1" );
        else if ( it->first == "baseurl" )
          info.addBaseUrl( Url(it->second) );
        else if ( it->first == "type" )
          info.setType(repo::RepoType(it->second));
      }
      
      // add it to the list.
      repos.push_back(info);
    }
    
    return repos;
  }
  
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
    std::list<RepoInfo> repos;
    list<Pathname> entries;
    if ( filesystem::readdir( entries, Pathname(dir), false ) != 0 )
      ZYPP_THROW(Exception("failed to read directory"));
    
    for ( list<Pathname>::const_iterator it = entries.begin(); it != entries.end(); ++it )
    {
      Pathname file = *it;
      std::list<RepoInfo> repos_here = repositories_in_file(file);
      std::copy( repos_here.begin(), repos_here.end(), std::back_inserter(repos));
    }
    return repos;
  }
  
  std::list<RepoInfo> RepoManager::knownRepositories()
  {
    return repositories_in_path("/etc/zypp/repos.d");
  }

  /** RepoManager implementation. */
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

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : RepoManager::RepoManager
  //	METHOD TYPE : Ctor
  //
  RepoManager::RepoManager( const RepoManagerOptions &opt )
  : _pimpl( new Impl(opt) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : RepoManager::~RepoManager
  //	METHOD TYPE : Dtor
  //
  RepoManager::~RepoManager()
  {}

  static void assert_alias( const RepoInfo &info )
  {
    if (info.alias().empty())
        ZYPP_THROW(RepoNoAliasException());
  }
  
  static void assert_urls( const RepoInfo &info )
  {
    if (info.urls().empty())
        ZYPP_THROW(RepoNoUrlException());
  }
  
  static Pathname rawcache_path_for_repoinfo( const RepoManagerOptions &opt, const RepoInfo &info )
  {
    assert_alias(info);
    return opt.repoRawCachePath + info.alias();
  }

  void RepoManager::refreshMetadata( const RepoInfo &info )
  {
    assert_alias(info);
    assert_urls(info);
    
    // try urls one by one
    for ( RepoInfo::urls_const_iterator it = info.urlsBegin(); it != info.urlsEnd(); ++it )
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
          YUMDownloader downloader( url, "/" );
          downloader.download(tmpdir.path());
           // no error
        }
        break;
        case RepoType::YAST2_e :
        {
          SUSETagsDownloader downloader( url, "/" );
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
  
  void RepoManager::cleanMetadata( const RepoInfo &info )
  {
    filesystem::recursive_rmdir(rawcache_path_for_repoinfo(_pimpl->options, info));
  }
  
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
  
  repo::RepoType RepoManager::probe( const Url &url )
  {
    MediaSetAccess access(url);
    if ( access.doesFileExist("/repodata/repomd.xml") )
      return repo::RepoType::RPMMD;
    if ( access.doesFileExist("/content") )
      return repo::RepoType::YAST2;
    
    return repo::RepoType("UNKNOWN");
  }
  
  void RepoManager::cleanCache( const RepoInfo &info )
  {
    cache::CacheStore store(_pimpl->options.repoCachePath);

    data::RecordId id = store.lookupRepository(info.alias());
    store.cleanRepository(id);
    store.commit();
  }
  
  Repository RepoManager::createFromCache( const RepoInfo &info )
  {
    cache::CacheStore store(_pimpl->options.repoCachePath);
    
    if ( ! store.isCached( info.alias() ) )
      ZYPP_THROW(RepoNotCachedException());
    
    MIL << "Repository " << info.alias() << " is cached" << endl;
    
    data::RecordId id = store.lookupRepository(info.alias());
    repo::cached::RepoImpl::Ptr repoimpl = new repo::cached::RepoImpl( info, _pimpl->options.repoCachePath, id );
    // read the resolvables from cache
    repoimpl->createResolvables();
    return Repository(repoimpl);
  }
 
  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const RepoManager & obj )
  {
    return str << *obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp2
///////////////////////////////////////////////////////////////////
