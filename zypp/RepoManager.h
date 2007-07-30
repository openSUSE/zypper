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
#include "zypp/Repository.h"
#include "zypp/RepoInfo.h"
#include "zypp/repo/RepoException.h"
#include "zypp/repo/RepoType.h"
#include "zypp/RepoStatus.h"
#include "zypp/ProgressData.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

   /**
    * Parses \a repo_file and returns a list of \ref RepoInfo objects
    * corresponding to repositories found within the file.
    * 
    * \param repo_file Valid URL of the repo file.
    * \return found list<RepoInfo>
    *
    * \throws MediaException If the access to the url fails
    * \throws ParseException If the file parsing fails
    * \throws Exception On other errors.
    */
   std::list<RepoInfo> readRepoFile(const Url & repo_file);

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
    
    enum RawMetadataRefreshPolicy
    {
      RefreshIfNeeded,
      RefreshForced
    };
    
    enum CacheBuildPolicy
    {
      BuildIfNeeded,
      BuildForced
    };
    
    enum RepoRemovePolicy
    {
      
    };

   /**
    * \short List known repositories.
    *
    * The known repositories are read from
    * \ref RepoManagerOptions::knownReposPath passed on the Ctor.
    * Which defaults to ZYpp global settings.
    * \return found list<RepoInfo>
    */
   std::list<RepoInfo> knownRepositories() const;

   /**
    * \short Status of local metadata
    */
    RepoStatus metadataStatus( const RepoInfo &info ) const;

   /**
    * \short Refresh local raw cache
    *
    * Will try to download the metadata
    *
    * In case of falure the metadata remains
    * as it was before.
    *
    * \throws repo::RepoNoUrlException if no urls are available.
    * \throws repo::RepoNoAliasException if can't figure an alias
    * \throws repo::RepoUnknownTypeException if the metadata is unknown
    * \throws repo::RepoException if the repository is invalid
    *         (no valid metadata found at any of baseurls) 
    * \throws Exception on unknown error.
    */
   void refreshMetadata( const RepoInfo &info,
                         RawMetadataRefreshPolicy policy = RefreshIfNeeded,
                         const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );
   
   /**
    * \short Clean local metadata
    *
    * Empty local metadata.
    *
    * \throws repo::RepoNoAliasException if can't figure an alias
    * \throws Exception on unknown error.
    */
   void cleanMetadata( const RepoInfo &info,
                       const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );

   /**
    * \short Status of metadata cache
    */
    RepoStatus cacheStatus( const RepoInfo &info ) const;
   
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
    * \throws repo::RepoNoAliasException if can't figure 
    * an alias to look in cache
    *
    * \throws repo::RepoMetadataException if the metadata
    * is not enough to build a cache (empty, incorrect, or
    * refresh needed)
    * \throws Exception on unknown error.
    */
   void buildCache( const RepoInfo &info,
                    CacheBuildPolicy policy = BuildIfNeeded,
                    const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );
   
   /**
    * \short clean local cache
    *
    * Clean the cached version of the metadata
    *
    * \note the local metadata must be valid.
    *
    * \throws repo::RepoNoAliasException if can't figure an alias to look in cache
    * \throws Exception on unknown error.
    */
   void cleanCache( const RepoInfo &info,
                    const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );
   
   /**
    * \short Whether a repository exists in cache
    *
    * \param RepoInfo to be checked.
    */
    bool isCached( const RepoInfo &info ) const;
   
   /**
    * \short Create a repository object from the cache data
    *
    * Creating from cache requires that the repository is
    * refreshed (metadata downloaded) and cached
    *
    * \throws repo::RepoNoAliasException if can't figure an alias to look in cache
    * \throw RepoNotCachedException When the source is not cached.
    */
   Repository createFromCache( const RepoInfo &info,
                               const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );

   /**
    * \short Create a repository object from raw metadata
    *
    * Creating from cache requires that the repository is
    * refreshed (metadata downloaded)
    *
    * \throw Exception If there are errors parsing the
    * raw metadata
    */
   Repository createFromMetadata( const RepoInfo &info,
                                  const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );

   /**
    * \short Probe repo metadata type.
    *
    * \todo FIXME Should this be private?
    */
   repo::RepoType probe( const Url &url ) const;
   
   
   /**
    * \short Adds a repository to the list of known repositories.
    *
    * 
    *
    * \throws repo::RepoAlreadyExistsException If the repo clash some 
    * unique attribute like alias
    *
    * \throws RepoAlreadyExistsException
    */
   void addRepository( const RepoInfo &info,
                       const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );
   
   /**
    * \short Adds repositores from a repo file to the list of known repositories.
    * \param url Url of the repo file
    * 
    * \throws repo::RepoAlreadyExistsException If the repo clash some 
    * unique attribute like alias
    *
    * \throws RepoAlreadyExistsException
    * \throws MediaException If the access to the url fails
    * \throws ParseException If the file parsing fails
    * \throws Exception On other errors.
    */
    void addRepositories( const Url &url,
                         const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );
    /**
     * \short Remove the best matching repository from known repos list
     *
     * \throws RepoNotFoundException If no repo match
     */
    void removeRepository( const RepoInfo & info,
                           const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );
    
    /**
     * \short Modify repository attributes
     *
     * \throws RepoNotFoundException If no repo match
     * \throws ParseException If the file parsing fails
     * \throws Exception On other errors.
     */
    void modifyRepository( const std::string &alias,
                           const RepoInfo & newinfo,
                           const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );

    /**
     * \short Find a matching repository info
     *
     * \note if multple repositories incorrectly share the
     * same alias, the first one found will be returned.
     *
     * \throws RepoNotFoundException If no repo match
     * \throws ParseException If the file parsing fails
     * \throws Exception On other errors.
     */
    RepoInfo getRepositoryInfo( const std::string &alias,
                                const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );
    
    /**
     * \short Find repository info by URL.
     *
     * \param url URL to find.
     * \param urlview url::ViewOption to influence URL matching.
     * \param progressrcv Progress receiver function.
     *
     * \note if multple repositories incorrectly share the
     * same URL, the first one found will be returned.
     *
     * \note the string representation of the URLs are compared.
     *       The \a urlview can be used to influence which
             parts of the URL are to be compared.
     *
     * \throws RepoNotFoundException If no repo match
     * \throws ParseException If the file parsing fails
     * \throws Exception On other errors.
     */
    RepoInfo getRepositoryInfo( const Url & url,
                                const url::ViewOption & urlview = url::ViewOption::DEFAULTS,
                                const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );

  protected:
    RepoStatus rawMetadataStatus( const RepoInfo &info );
    RepoStatus cacheStatus( const RepoInfo &info );
  public:

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoManager Stream output */
  std::ostream & operator<<( std::ostream & str, const RepoManager & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP2_REPOMANAGER_H
