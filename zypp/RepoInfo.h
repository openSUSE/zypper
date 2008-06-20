/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/RepoInfo.h
 *
*/
#ifndef ZYPP2_REPOSITORYINFO_H
#define ZYPP2_REPOSITORYINFO_H

#include <iosfwd>
#include <list>
#include <set>
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Iterator.h"
#include "zypp/base/Deprecated.h"

#include "zypp/Pathname.h"
#include "zypp/Url.h"
#include "zypp/repo/RepoType.h"
#include "zypp/repo/RepoVariables.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoInfo
  //
  /**
   * \short What is known about a repository
   *
   * The class RepoInfo represents everything that
   * is known about a software repository.
   *
   * It can be used to store information about known
   * sources.
   *
   * This class tries to be compatible with the
   * concept of a .repo file used by YUM and
   * also available in the openSUSE build service.
   * See <tt>man yum.conf</tt>.
   *
   * Example file
   *
   * \code
   * [ruby]
   * name=Ruby repository (openSUSE_10.2)
   * type=rpm-md
   * baseurl=http://software.opensuse.org/download/ruby/openSUSE_10.2/
   * gpgcheck=1
   * gpgkey=http://software.opensuse.org/openSUSE-Build-Service.asc
   * enabled=1
   * priority=10
   * \endcode
   *
   * \note A RepoInfo is a hint about how
   * to create a Repository.
   */
  class RepoInfo
  {
    friend std::ostream & operator<<( std::ostream & str, const RepoInfo & obj );

    public:
    static unsigned defaultPrioity();

    public:
    RepoInfo();
    ~RepoInfo();

    /**
     * unique identifier for this source. If not specified
     * It should be generated from the base url.
     *
     * Normally, in a .repo file the section name is used
     * ( [somerepo] )
     */
    std::string alias() const;

    /**
     * Same as alias(), just escaped in a way to be a valid file name.
     */
    std::string escaped_alias() const;

    /**
     * Repository priority for solver.
     * Some number between \c 1 (highest priority) and \c 99 (\ref defaultPriority).
     */
    unsigned priority() const;
    /**
     * The default priority (\c 99).
     */
    static unsigned defaultPriority();
    /**
     * Set repository priority for solver.
     * A \c newval_r of \c 0 sets the default priority.
     * \see \ref priority.
     */
    RepoInfo & setPriority( unsigned newval_r );

    /**
     * A Url under which the metadata are located, or a set of mirrors.
     *
     * This can't be empty in order the repository to be valid
     * unless the download of the mirror list succeeds and it
     * contains a valid url.
     *
     * \deprecated IMO superfluous as we provide begin/end iterator.
     */
    ZYPP_DEPRECATED std::set<Url> baseUrls() const;

    /**
     * \short Repository path
     *
     * Pathname relative to the base Url where the product/repository
     * is located
     *
     * For medias containing more than one product, or repositories not
     * located at the root of the media it is important to
     * know the path of the media root relative to the product directory
     * so a media verifier can be set for that media.
     *
     * It is not mandatory, and the default is /
     *
     * \note As a repository can have multiple Urls, the path is unique and
     * the same for all Urls, so it is assumed all the Urls have the
     * same media layout.
     *
     */
    Pathname path() const;

    /**
     * Url of a file which contains a list of Urls
     * If empty, the base url will be used.
     */
    Url mirrorListUrl() const;

    typedef std::set<Url> url_set;
    //typedef url_set::const_iterator urls_const_iterator;
    typedef url_set::size_type      urls_size_type;
    typedef transform_iterator<repo::RepoVariablesUrlReplacer, url_set::const_iterator> urls_const_iterator;

    /**
     * iterator that points at begin of repository urls
     */
    urls_const_iterator baseUrlsBegin() const;

    /**
     * iterator that points at end of repository urls
     */
    urls_const_iterator baseUrlsEnd() const;

    /**
     * number of repository urls
     */
    urls_size_type baseUrlsSize() const;

     /**
      * whether repository urls are available
      */
    bool baseUrlsEmpty() const;

   /**
     * If enabled is false, then this repository must be ignored as if does
     * not exists, except when checking for duplicate alias.
     */
    bool enabled() const;

    /**
     * If true, the repostory must be refreshed before creating resolvables
     * from it
     */
    bool autorefresh() const;

    /**
     * Type of repository,
     *
     */
    repo::RepoType type() const;

    /**
     * \short Repository short label
     *
     * Short label or description of the repository, to be used on
     * the user interface.
     * ie: "SUSE Linux 10.2 updates"
     */
    std::string name() const;

    /**
     * \short File where this repo was read from
     *
     * \note could be an empty pathname for repo
     * infos created in memory.
     */
     Pathname filepath() const;

     /**
     * \short Path where this repo metadata was read from
     *
     * \note could be an empty pathname for repo
     * infos created in memory.
     */
     Pathname metadataPath() const;

     /**
     * \short Path where this repo packages are cached
     */
     Pathname packagesPath() const;

     /**
     * \short Whether to check or not this repository with gpg
     *
     * \note This is a just a hint to the application and can
     * be ignored.
     *
     */
    bool gpgCheck() const;

    /**
     * \short Key to use for gpg checking of this repository
     *
     * \param url Url to the key in ASCII armored format
     *
     * \note This is a just a hint to the application and can
     * be ignored.
     *
     */
     Url gpgKeyUrl() const;

    /**
     * \short Whether to keep the packages downloaded from this repository will be kept in local cache
     */
    bool keepPackages() const;

    /**
     * Gets service name which add it or empty string if this repository
     * is added manually.
     */
    std::string service() const;

    public:
    /**
     * Add a base url. \see baseUrls
     * \param url The base url for the repository.
     * \note can change keepPackages,so change it after this call
     *
     * To recreate the base URLs list, use \ref setBaseUrl(const Url &) followed
     * by addBaseUrl().
     */
    RepoInfo & addBaseUrl( const Url &url );

    /**
     * Clears current base URL list and adds \a url.
     * \note can change keepPackages,so change it after this call
     */
    RepoInfo & setBaseUrl( const Url &url );

    /**
     * set the product path. \see path()
     * \param path the path to the product
     */
    RepoInfo & setPath( const Pathname &path );

    /**
     * Set mirror list url. \see mirrorListUrl
     * \param url The base url for the list
     */
    RepoInfo & setMirrorListUrl( const Url &url );

    /**
     * enable or disable the repository \see enabled
     * \param enabled
     */
    RepoInfo & setEnabled( bool enabled );

    /**
     * enable or disable autorefresh \see autorefresh
     * \param enabled
     */
    RepoInfo & setAutorefresh( bool autorefresh );

    /**
     * set the repository alias \see alias
     * \param alias
     */
    RepoInfo & setAlias( const std::string &alias );

    /**
     * set the repository type \see type
     * \param t
     */
    RepoInfo & setType( const repo::RepoType &t );

    /**
     * This allows to adjust the \ref  RepoType lazy, from \c NONE to
     * some probed value, even for const objects.
     *
     * This is a NOOP if the current type is not \c NONE.
     */
    void setProbedType( const repo::RepoType &t ) const;

    /**
     * set the repository name \see name
     * \param name
     */
    RepoInfo & setName( const std::string &name );

    /**
     * \short set the path to the .repo file
     *
     * The path to the .repo file where this repository
     * was defined, or empty if nowhere.
     *
     * \param path File path
     */
    RepoInfo & setFilepath( const Pathname &filename );

    /**
     * \short set the path where the local metadata is stored
     *
     * The path to the metadata of this repository
     * was defined, or empty if nowhere.
     *
     * \param path directory path
     */
    RepoInfo & setMetadataPath( const Pathname &path );

    /**
     * \short set the path where the local packages are stored
     *
     * \param path directory path
     */
    RepoInfo & setPackagesPath( const Pathname &path );

    /**
     * \short Whether to check or not this repository with gpg
     *
     * \param check true (check) or false (dont'check)
     *
     * \note This is a just a hint to the application and can
     * be ignored.
     *
     */
    RepoInfo & setGpgCheck( bool check );

    /**
     * \short Key to use for gpg checking of this repository
     *
     * \param url Url to the key in ASCII armored format
     *
     * \note This is a just a hint to the application and can
     * be ignored.
     *
     */
    RepoInfo & setGpgKeyUrl( const Url &gpgkey );

    /**
     * \short Set if the packaqes downloaded from this repository will be kept in local cache
     *
     * If the setting is true, all downloaded packages from this repository will be
     * copied to the local raw cache.
     *
     * \param keep true (keep the downloaded packages) or false (delete them after installation)
     *
     */
    RepoInfo & setKeepPackages( bool keep );

    /**
     * sets service which added this repository
     */
    RepoInfo & setService( const std::string& name );

    /**
     * Write a human-readable representation of this RepoInfo object
     * into the \a str stream. Useful for logging.
     */
    std::ostream & dumpOn( std::ostream & str ) const;

    /**
     * Write this RepoInfo object into \a str in a <tr>.repo</tt> file format.
     */
    std::ostream & dumpRepoOn( std::ostream & str ) const;

    class Impl;
  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoInfo Stream output */
  std::ostream & operator<<( std::ostream & str, const RepoInfo & obj );

  inline bool operator<( const RepoInfo& lhs, const RepoInfo & rhs )
  { return lhs.alias() < rhs.alias(); }

  typedef std::list<RepoInfo> RepoInfoList;

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP2_REPOSITORYINFO_H
