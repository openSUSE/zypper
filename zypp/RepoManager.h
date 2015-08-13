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
#include "zypp/base/Iterator.h"
#include "zypp/base/Flags.h"

#include "zypp/Pathname.h"
#include "zypp/ZConfig.h"
#include "zypp/RepoInfo.h"
#include "zypp/repo/RepoException.h"
#include "zypp/repo/RepoType.h"
#include "zypp/repo/ServiceType.h"
#include "zypp/ServiceInfo.h"
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
    Pathname pluginsPath;
    bool probe;
    /**
     * Target distro ID to be used when refreshing repo index services.
     * Repositories not maching this ID will be skipped/removed.
     *
     * If empty, \ref Target::targetDistribution() will be used instead.
     */
    std::string servicesTargetDistro;

    /** remembers root_r value for later use */
    Pathname rootDir;
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

    /** ServiceInfo typedefs */
    typedef std::set<ServiceInfo> ServiceSet;
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

    /** Flags for tuning RefreshService */
    enum RefreshServiceBit
    {
      RefreshService_restoreStatus	= (1<<0),	///< Force restoring repo enabled/disabled status
      RefreshService_forceRefresh	= (1<<1),	///< Force refresh even if TTL is not reached
    };
    ZYPP_DECLARE_FLAGS(RefreshServiceFlags,RefreshServiceBit);

    /** Options tuning RefreshService */
    typedef RefreshServiceFlags RefreshServiceOptions;


    /** \name Known repositories.
     *
     * The known repositories are read from
     * \ref RepoManagerOptions::knownReposPath passed on the Ctor.
     * Which defaults to ZYpp global settings.
     */
   //@{
    bool repoEmpty() const;
    RepoSizeType repoSize() const;
    RepoConstIterator repoBegin() const;
    RepoConstIterator repoEnd() const;

    /** List of known repositories. */
    std::list<RepoInfo> knownRepositories() const
    { return std::list<RepoInfo>(repoBegin(),repoEnd()); }

    /** Find RepoInfo by alias or return \ref RepoInfo::noRepo. */
    RepoInfo getRepo( const std::string & alias ) const;
    /** \overload Take alias from RepoInfo. */
    RepoInfo getRepo( const RepoInfo & info_r ) const
    { return getRepo( info_r.alias() ); }

    /** Return whether there is a known repository for \c alias. */
    bool hasRepo( const std::string & alias ) const;
    /** \overload Take alias from RepoInfo. */
    bool hasRepo( const RepoInfo & info_r ) const
    { return hasRepo( info_r.alias() ); }

    /** Some stupid string but suitable as alias for your url if nothing better is available.
     * Something like \c "http-download.opensuse.org-83df67e5"
    */
    static std::string makeStupidAlias( const Url & url_r = Url() );
   //@}

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
    * Remove any subdirectories of cache directories which no longer belong
    * to any of known repositories.
    *
    * These can be temporary directories left by interrupted refresh,
    * or dirs left after changing .repo files outside of libzypp.
    */
   void cleanCacheDirGarbage( const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );

   /**
    * \short Probe repo metadata type.
    *
    * The location to probe consists of the base \a url (you may think of it as
    * a mountpoint) and the \a path to the repository on the mounted media
    * (ususally \c / ).
    */
   repo::RepoType probe( const Url & url, const Pathname & path ) const;
   /**
    * \overload Using the default path \c "/".
    */
   repo::RepoType probe( const Url & url ) const;


   /**
    * \short Adds a repository to the list of known repositories.
    *
    *
    *
    * \throws repo::RepoAlreadyExistsException If the repo clash some
    *         unique attribute like alias
    * \throws RepoUnknownType
    *         If RepoManagerOptions::probe is true
    *         and repository type can't be determined.
    * \throws RepoException
    *         If RepoManagerOptions::probe is true and access to the url fails.
    * \throws Exception On other errors.
    */
   void addRepository( const RepoInfo &info,
                       const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() );

   /**
    * \short Adds repositores from a repo file to the list of known repositories.
    * \param url Url of the repo file
    *
    * \throws repo::RepoAlreadyExistsException If the repo clash some
    *         unique attribute like alias
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
    /** \overload Take alias from RepoInfo. */
    void modifyRepository( const RepoInfo & newinfo,
                           const ProgressData::ReceiverFnc & progressrcv = ProgressData::ReceiverFnc() )
    { modifyRepository( newinfo.alias(), newinfo, progressrcv ); }

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


    /** \name Known services.
     *
     * The known services are read from
     * \ref RepoManagerOptions::knownServicesPath passed on the Ctor.
     * Which defaults to ZYpp global settings.
     */
    //@{
    /**
     * Gets true if no service is in RepoManager (so no one in specified location)
     *
     * \return true if any ServiceInfo is in RepoManager
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
     * \note Iterator is immutable, so you cannot change pointed ServiceInfo
     * \return Iterator to first service
     */
    ServiceConstIterator serviceBegin() const;

    /**
     * Iterator to place behind last service in internal storage.
     * \return iterator to end
     */
    ServiceConstIterator serviceEnd() const;

    /** List of known services. */
    std::list<ServiceInfo> knownServices() const
    { return std::list<ServiceInfo>(serviceBegin(),serviceEnd()); }

    /**
     * \short Finds ServiceInfo by alias or return \ref ServiceInfo::noService
     *
     * \param alias unique identifier of service
     * \return information about service
     */
    ServiceInfo getService( const std::string & alias ) const;

    /** Return whether there is a known service for \c alias. */
    bool hasService( const std::string & alias ) const;
    //@}

    /**
     * \short Probe the type or the service.
     */
    repo::ServiceType probeService( const Url &url ) const;

    /**
     * Adds new service by it's alias and url
     *
     * \param alias unique identifier of the service
     * \param url url to service
     *
     * \throws FIXME RepoAlreadyExistException and as reponame is service name
     */
    void addService( const std::string & alias, const Url& url );

    /**
     * Adds new service
     *
     * \param service service info
     *
     * \throws FIXME RepoAlreadyExistException and as reponame is service name
     */
    void addService( const ServiceInfo & service );

    /**
     * Removes service specified by its name
     *
     * \param alias unique indientifier of the service to remove
     *
     * \throws RepoException if service is not found or file with ServiceInfo cannot be deleted
     * \throws Exception if file contain more services and rewrite file failed
     */
    void removeService( const std::string & alias );
    /** \overload Take alias from ServiceInfo */
    void removeService( const ServiceInfo & service );


    /**
     * Refreshes all enabled services.
     *
     * \see refreshService(ServiceInfo)
     */
    void refreshServices( const RefreshServiceOptions & options_r = RefreshServiceOptions() );

    /**
     * Refresh specific service.
     *
     * \param alias unique indientifier of the service to refresh
     *
     * \throws RepoException if service is not found.
     * \throws MediaException If there's a problem downloading the repo index file.
     */
    void refreshService( const std::string & alias, const RefreshServiceOptions & options_r = RefreshServiceOptions() );
    /** \overload Take alias from ServiceInfo */
    void refreshService( const ServiceInfo & service, const RefreshServiceOptions & options_r = RefreshServiceOptions() );

    /**
     * Modifies service file (rewrites it with new values) and underlying
     * repositories if needed.
     *
     * Modifications of a service can lead to rewrite of all .repo files of
     * contained repositories. Particularily, disabling a service (changing
     * ServiceInfo::enabled() from true to false) will disable all contained
     * repositories. Renaming of a service will modify the "service" key
     * of all contained repositories.
     *
     * \param oldAlias Old alias of the service
     * \param service ServiceInfo object containing new data
     *
     * \throws RepoException if sservice with oldAlias is not known
     * \throws Exception if have problems with files
     */
    void modifyService( const std::string & oldAlias, const ServiceInfo & service );
    /** \overload Take alias from ServiceInfo. */
    void modifyService( const ServiceInfo & service )
    { modifyService( service.alias(), service ); }

  private:
    /**
     * Functor thats filter RepoInfo by service which it belongs to.
     */
    struct MatchServiceAlias
    {
      public:
        MatchServiceAlias( const std::string & alias_ ) : alias(alias_) {}
        bool operator()( const RepoInfo & info ) const
        { return info.service() == alias; }
      private:
        std::string alias;
    };

  public:

    /**
     * fill to output iterator repositories in service name. This output iterator can perform
     * any action on with Repo or service Container, because it is sets and it isn't dynamic recreate.
     *
     * \note Don't use this function with RepoManager::removeRepository(), it will lead to segfaults
     *       due to invalidated internal iterators. FIXME can this be solved (using STL) so that this
     *       warning would not be needed?
     *
     * \param alias service alias
     * \param out output iterator which get all the repositories belonging to
     *   specified service
     *
     * example how set priority for each RepoInfo in this service:
     * \code
     * //functor
     * class ChangePriority
     * {
     * private:
     *   int priority;
     * public:
     *   ChangePriority(int prio) : priority(prio) {}
     *   // missing rewrite priority back via RepoManager::modifyRepo
     *   void doIt( RepoInfo info ) { info.setPriority(priority); }
     * }
     *
     * //somewhere in code
     * ChangePriority changer(10);
     * getRepositoriesInService(name,
     *   boost::make_function_output_iterator(
     *     bind(&ChangePriority::doIt, &changer, _1)));
     * \endcode
     */
    template<typename OutputIterator>
    void getRepositoriesInService( const std::string & alias,
                                   OutputIterator out ) const
    {
      MatchServiceAlias filter(alias);

      std::copy( boost::make_filter_iterator( filter, repoBegin(), repoEnd() ),
                 boost::make_filter_iterator( filter, repoEnd(), repoEnd() ),
                 out);
    }

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ZYPP_DECLARE_OPERATORS_FOR_FLAGS(RepoManager::RefreshServiceFlags);
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoManager Stream output */
  std::ostream & operator<<( std::ostream & str, const RepoManager & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP2_REPOMANAGER_H
