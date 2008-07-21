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

#include <boost/iterator.hpp>

#include "zypp/base/PtrTypes.h"
#include "zypp/Pathname.h"
#include "zypp/ZConfig.h"
#include "zypp/RepoInfo.h"
#include "zypp/repo/RepoException.h"
#include "zypp/repo/RepoType.h"
#include "zypp/RepoStatus.h"
#include "zypp/ProgressData.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  class Service; //predef

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
    /** Default ctor following \ref ZConfig global settings.
     * If an optional \c root_r directory is given, all paths  will
     * be prefixed accordingly.
     * \code
     *    root_r\repoCachePath
     *          \repoRawCachePath
     *          \repoSolvCachePath
     *          \repoPackagesCachePath
     *          \knownReposPath
     * \endcode
     */
    RepoManagerOptions( const Pathname & root_r = Pathname() );

    /** Test setup adjusting all paths to be located below one \c root_r directory.
     * \code
     *    root_r\          - repoCachePath
     *          \raw       - repoRawCachePath
     *          \solv      - repoSolvCachePath
     *          \packages  - repoPackagesCachePath
     *          \repos.d   - knownReposPath
     * \endcode
     */
    static RepoManagerOptions makeTestSetup( const Pathname & root_r );

    Pathname repoCachePath;
    Pathname repoRawCachePath;
    Pathname repoSolvCachePath;
    Pathname repoPackagesCachePath;
    Pathname knownReposPath;
    Pathname knownServicesPath;
    bool probe;
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

    /** service typedefs */
    typedef std::set<Service> ServiceSet;
    typedef ServiceSet::const_iterator ServiceConstIterator;
    typedef ServiceSet::size_type ServiceSizeType;

    /** RepoInfo typedefs */
    typedef std::set<RepoInfo> RepoSet;
    typedef RepoSet::const_iterator RepoConstIterator;
    typedef RepoSet::size_type RepoSizeType;

  public:
   RepoManager( const RepoManagerOptions &options = RepoManagerOptions() );
   /** Dtor */
    ~RepoManager();

    enum RawMetadataRefreshPolicy
    {
      RefreshIfNeeded,
      RefreshForced,
      RefreshIfNeededIgnoreDelay
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
    * \deprecated use iterator instead which read only one time directory
    * \return found list<RepoInfo>
    */
   ZYPP_DEPRECATED std::list<RepoInfo> knownRepositories() const;

   bool repoEmpty() const;
   RepoSizeType repoSize() const;
   RepoConstIterator repoBegin() const;
   RepoConstIterator repoEnd() const;


   /**
    * \short Status of local metadata
    */
    RepoStatus metadataStatus( const RepoInfo &info ) const;

    /**
     * Possibly return state of checkIfRefreshMEtadata function
     */
    enum RefreshCheckStatus {
      REFRESH_NEEDED,  /**< refresh is needed */
      REPO_UP_TO_DATE, /**< repository not changed */
      REPO_CHECK_DELAYED     /**< refresh is delayed due to settings */
    };

    /**
     * Checks whether to refresh metadata for specified repository and url.
     * <p>
     * The need for refresh is evaluated according to the following conditions,
     * in that order:
     * <ul>
     * <li>the refresh policy (refresh may be forced)
     * <li>the repo.refresh.delay ZConfig value compared to the difference between
     *   cached index file timestamp and actual time
     * <li>the timestamp of cached repo index file compared to the remote
     *   index file timestamp.
     * </ul>
     * <p>
     * This method checks the status against the specified url only. If more
     * baseurls are defined for in the RepoInfo, each one must be check
     * individually. Example:
     *
     * <code>
     *
     * RepoInfo info;
     * // try urls one by one
     * for ( RepoInfo::urls_const_iterator it = info.baseUrlsBegin();
     *       it != info.baseUrlsEnd(); ++it )
     * {
     *   try
     *   {
     *     // check whether to refresh metadata
     *     // if the check fails for this url, it throws, so another url will be checked
     *     if (checkIfToRefreshMetadata(info, *it, policy)!=RepoInfo::REFRESH_NEEDED)
     *       return;
     *
     *     // do the actual refresh
     *   }
     *   catch (const Exception & e)
     *   {
     *     ZYPP_CAUGHT(e);
     *     ERR << *it << " doesn't look good. Trying another url." << endl;
     *   }
     * } // for all urls
     *
     * handle("No more URLs.");
     *
     * </code>
     *
     * \param info
     * \param url
     * \param policy
     * \return state of repository
     * \see RefreshCheckStatus
     * \throws RepoUnknownTypeException
     * \throws repo::RepoNoAliasException if can't figure an alias
     * \throws Exception on unknown error
     *
     */
    RefreshCheckStatus checkIfToRefreshMetadata( const RepoInfo &info,
                                   const Url &url,
                                   RawMetadataRefreshPolicy policy = RefreshIfNeeded);

    /**
     * \short Path where the metadata is downloaded and kept
     *
     * Given a repoinfo, tells where \ref RepoManager will download
     * and keep the raw metadata.
     *
     * \param info Repository information
     *
     * \throws repo::RepoNoAliasException if can't figure an alias
     */
    Pathname metadataPath( const RepoInfo &info ) const;


    /**
     * \short Path where the rpm packages are downloaded and kept
     *
     * Given a repoinfo, tells where \ref RepoProvidePackage will download
     * and keep the .rpm files.
     *
     * \param info Repository information
     *
     * \throws repo::RepoNoAliasException if can't figure an alias
     */
    Pathname packagesPath( const RepoInfo &info ) const;


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
    * \short Clean local package cache
    *
    * Empty local directory with downloaded packages
    *
    * \throws repo::RepoNoAliasException if can't figure an alias
    * \throws Exception on unknown error.
    */
   void cleanPackages( const RepoInfo &info,
                       const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );

   /**
    * \short Status of metadata cache
    */
    RepoStatus cacheStatus( const RepoInfo &info ) const;

   /**
    * \short Refresh local cache
    *
    * Will try to build the cache from local metadata.
    *
    * If the cache exists it will be overwriten.
    *
    * \note the local metadata must be valid.
    *
    * \throws repo::RepoNoAliasException if can't figure
    *     an alias to look in cache
    * \throws repo::RepoMetadataException if the metadata
    *     is not enough to build a cache (empty, incorrect, or
    *     refresh needed)
    * \throws repo::RepoUnknownTypeException
    * \throws parser::ParseException if parser encounters an error.
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
    * \throws cache::CacheRecordNotFoundException if the cache could not be
    *     cleaned because of repository record not found.
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
    * \short Load resolvables into the pool
    *
    * Creating from cache requires that the repository is
    * refreshed (metadata downloaded) and cached
    *
    * \throws repo::RepoNoAliasException if can't figure an alias to look in cache
    * \throw RepoNotCachedException When the source is not cached.
    */
   void loadFromCache( const RepoInfo &info,
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
    *         unique attribute like alias
    * \throws RepoUnknownType If repository type can't be determined
    * \throws RepoException If the access to the url fails (while probing).
    * \throws Exception On other errors.
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
    * \throws RepoUnknownType If repository type can't be determined
    * \throws RepoException ON other repository related errors
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
     * \throws RepoAlreadyExistsException if the alias specified in newinfo
     *         is already used by another repository
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
     * \note if multiple repositories incorrectly share the
     * same alias, the first one found will be returned.
     *
     * \param alias Repository alias
     * \param progressrcv Progress reporting function
     * \return RepoInfo of the found repository
     * \throws RepoNotFoundException If no repo match the alias
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
     * \return RepoInfo of the found repository.
     *
     * \note if multpile repositories incorrectly share the
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

    /**
     * Adds new service by it's name and url
     *
     * \param name unique name of service
     * \param url url to service
     *
     * \throws FIXME RepoAlreadyExistException and as reponame is service name
     */
    void addService( const std::string& name, const Url& url );

    /**
     * Adds new service
     *
     * \param name service info
     *
     * \throws FIXME RepoAlreadyExistException and as reponame is service name
     */
    void addService( const Service& name );

    /**
     * Removes service specified by its name
     *
     * \param name name of service to remove
     *
     * \throws RepoException if service is not found or file with Service cannot be deleted
     * \throws Exception if file contain more services and rewrite file failed
     */
    void removeService( const std::string& name );

    /**
     * Gets true if no service is in RepoManager (so no one in specified location)
     *
     * \return true if any Service is in RepoManager
     */
    bool serviceEmpty() const;

    /**
     * Gets count of service in RepoManager (in specified location)
     *
     * \return count of service
     */
    ServiceSizeType serviceSize() const;

    /**
     * Iterator to first service in internal storage.
     * \note Iterator is immutable, so you cannot change pointed Service
     * \return Iterator to first service
     */
    ServiceConstIterator serviceBegin() const;

    /**
     * Iterator to place behind last service in internal storage.
     * \return iterator to end
     */
    ServiceConstIterator serviceEnd() const;

    /**
     * Finds Service by name or return noService
     *
     * \param name unique name of service
     * \return information about Service
     */
    Service getService( const std::string& name ) const;

    /**
     * Refreshs all services
     * \see refreshService
     */
    void refreshServices();

    /**
     * Refresh specific service.
     * \throws Exception if cannot download file
     * \param name service structure
     */
    void refreshService( const Service& name );

    /**
     * modify service, rewrite Service to filesystem.
     * If change Service name, then can escalate to rewrite all repositories which it contain.
     *
     * \param oldName oldName of service
     * \param service Structure containing new datas
     *
     * \throws RepoException if sservice with oldName is not known
     * \throws Exception if have problems with files
     */
    void modifyService(const std::string& oldName, const Service& service);

    private:
    /**
     * Functor thats filter RepoInfo by service which belongs to.
     */
    struct MatchServiceName {
      private:
        std::string name;
      public:
        MatchServiceName( const std::string& name_ ) : name(name_) {}
        bool match( const RepoInfo& info ) { return info.service()==name; }
    };

    public:
    /**
     * fill to output iterator repositories in service name. This output iterator can perform
     * any action on with Repo or service Container, because it is sets and it isn't dynamic recreate.
     * \param name service name
     * \param out output iterator which get all repositories in Service
     *
     * example how set priority for each RepoInfo in this service:
     * \code
     * //functor
     * class ChangePriority{
     *   private:
     *     int priority;
     *   public:
     *     ChangePriority(int prio) : priority(prio) {}
     *     void doIt( RepoInfo info ) { info.setPriority(priority); } //missing rewrite priority back via RepoManager::modifyRepo
     * }
     *
     * //somewhere in code
     * ChangePriority changer(10);
     * getRepositoriesInService(name,getRepositoriesInService( name, boost::make_function_output_iterator(bind(&ChangePriority::doIt, &changer, _1))));
     * \endcode
     */

    template<typename OutputIterator>
    void getRepositoriesInService( const std::string& name, OutputIterator out ) const
    {
      MatchServiceName filter(name);

      std::copy(boost::make_filter_iterator(bind(&MatchServiceName::match,
          filter, _1),repoBegin(),repoEnd()), boost::make_filter_iterator(
          bind(&MatchServiceName::match, filter, _1),repoEnd(),repoEnd()),
          out );
    }

  protected:
    RepoStatus rawMetadataStatus( const RepoInfo &info );
    void setCacheStatus( const RepoInfo &info, const RepoStatus &status );

    /**
     * Update timestamp of repository index file for the specified repository \a info.
     * Used in \ref checkIfToRefreshMetadata() for repo.refresh.delay feature.
     */
    void touchIndexFile(const RepoInfo & info);

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
