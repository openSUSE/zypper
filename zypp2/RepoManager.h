/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/RepoManager.h
 *
*/
#ifndef ZYPP_REPOMANAGER_H
#define ZYPP_REPOMANAGER_H

#include <iosfwd>
#include <list>

#include "zypp/base/PtrTypes.h"
//#include "zypp/base/ReferenceCounted.h"
//#include "zypp/base/NonCopyable.h"
#include "zypp/Pathname.h"
#include "zypp/ZConfig.h"
#include "zypp2/Repository.h"
#include "zypp2/RepoInfo.h"
#include "zypp2/repo/RepoException.h"
#include "zypp2/repo/RepoType.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /**
   * Repo manager settings.
   * Settings default to ZYpp global settings.
   */
  struct RepoManagerOptions
  {
    RepoManagerOptions();
    
    Pathname repoCachePath;
    Pathname repoRawCachePath;
    Pathname knownReposPath;
  };

  /**
   * \short creates and provides information about known sources.
   *
   */
  class RepoManager
  {
    friend std::ostream & operator<<( std::ostream & str, const RepoManager & obj );

  public:
    /** Implementation  */
    class Impl;

  public:
   RepoManager( const RepoManagerOptions &options = RepoManagerOptions() );
   /** Dtor */
    ~RepoManager();
    
   /**
    * \short List known repositories.
    *
    * The known repositories are read from
    * \ref RepoManagerOptions::knownReposPath passed on the Ctor.
    * Which defaults to ZYpp global settings.
    *
    */
   std::list<RepoInfo> knownRepositories();
   
   /**
    * \short Refresh local cache
    *
    * Will try to download the metadata
    *
    * In case of falure the metadata remains
    * as it was before.
    *
    * \throws RepoNoUrlException if no urls are available.
    * \throws RepoNoAliasException if can't figure an alias
    * \throws RepoUnknownTypeException if the metadata is unknown
    * \throws Exception on unknown error.
    */
   void refreshMetadata( const RepoInfo &info );
   
   /**
    * \short Clean local metadata
    *
    * Empty local metadata.
    *
    * \throws RepoNoAlias if can't figure an alias
    * \throws Exception on unknown error.
    */
   void cleanMetadata( const RepoInfo &info );
   
   /**
    * \short Refresh local cache
    *
    * Will try to build the cache from
    * local metadata.
    *
    * If the cache exists it will be overwriten.
    *
    * \note the local metadata must be valid.
    *
    * \throws RepoNoAlias if can't figure an alias to look in cache
    * \throws Exception on unknown error.
    */
   void buildCache( const RepoInfo &info );
   
   /**
    * \short clean local cache
    *
    * Clean the cached version of the metadata
    *
    * \note the local metadata must be valid.
    *
    * \throws RepoNoAlias if can't figure an alias to look in cache
    * \throws Exception on unknown error.
    */
   void cleanCache( const RepoInfo &info );
   
   /**
    * \short Create a repository object from the cache data
    *
    * \throw RepoNotCachedException When the source is not cached.
    */
   Repository createFromCache( const RepoInfo &info );

   
   /**
    * \short Probe repo metadata type.
    *
    * \todo FIXME Should this be private?
    */
   repo::RepoType probe( const Url &url );
   
  public:

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoManager Stream output */
  std::ostream & operator<<( std::ostream & str, const RepoManager & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp2
///////////////////////////////////////////////////////////////////
#endif // ZYPP2_REPOMANAGER_H
