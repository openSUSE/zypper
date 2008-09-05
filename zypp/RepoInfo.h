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

#include <list>
#include <set>

#include "zypp/base/Iterator.h"
#include "zypp/base/Deprecated.h"

#include "zypp/Url.h"
#include "zypp/repo/RepoType.h"
#include "zypp/repo/RepoVariables.h"

#include "zypp/repo/RepoInfoBase.h"

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
  class RepoInfo : public repo::RepoInfoBase
  {
    friend std::ostream & operator<<( std::ostream & str, const RepoInfo & obj );

    public:
    RepoInfo();
    ~RepoInfo();

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
    void setPriority( unsigned newval_r );

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
     * Type of repository,
     *
     */
    repo::RepoType type() const;

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
     * Gets name of the service to which this repository belongs or empty string
     * if it has been added manually.
     */
    std::string service() const;

    /**
     * Distribution for which is this repository meant.
     */
    std::string targetDistribution() const;

    public:
    /**
     * Add a base url. \see baseUrls
     * \param url The base url for the repository.
     * \note can change keepPackages,so change it after this call
     *
     * To recreate the base URLs list, use \ref setBaseUrl(const Url &) followed
     * by addBaseUrl().
     */
    void addBaseUrl( const Url &url );

    /**
     * Clears current base URL list and adds \a url.
     * \note can change keepPackages,so change it after this call
     */
    void setBaseUrl( const Url &url );

    /**
     * set the product path. \see path()
     * \param path the path to the product
     */
    void setPath( const Pathname &path );

    /**
     * Set mirror list url. \see mirrorListUrl
     * \param url The base url for the list
     */
    void setMirrorListUrl( const Url &url );

    /**
     * set the repository type \see type
     * \param t
     */
    void setType( const repo::RepoType &t );

    /**
     * This allows to adjust the \ref  RepoType lazy, from \c NONE to
     * some probed value, even for const objects.
     *
     * This is a NOOP if the current type is not \c NONE.
     */
    void setProbedType( const repo::RepoType &t ) const;

    /**
     * \short set the path where the local metadata is stored
     *
     * The path to the metadata of this repository
     * was defined, or empty if nowhere.
     *
     * \param path directory path
     */
    void setMetadataPath( const Pathname &path );

    /**
     * \short set the path where the local packages are stored
     *
     * \param path directory path
     */
    void setPackagesPath( const Pathname &path );

    /**
     * \short Whether to check or not this repository with gpg
     *
     * \param check true (check) or false (dont'check)
     *
     * \note This is a just a hint to the application and can
     * be ignored.
     *
     */
    void setGpgCheck( bool check );

    /**
     * \short Key to use for gpg checking of this repository
     *
     * \param url Url to the key in ASCII armored format
     *
     * \note This is a just a hint to the application and can
     * be ignored.
     *
     */
    void setGpgKeyUrl( const Url &gpgkey );

    /**
     * \short Set if the packaqes downloaded from this repository will be kept in local cache
     *
     * If the setting is true, all downloaded packages from this repository will be
     * copied to the local raw cache.
     *
     * \param keep true (keep the downloaded packages) or false (delete them after installation)
     *
     */
    void setKeepPackages( bool keep );

    /**
     * sets service which added this repository
     */
    void setService( const std::string& name );

    /**
     * Sets the distribution for which is this repository meant. This is
     * an in-memory value only, does not get written to the .repo file upon
     * saving.
     */
    void setTargetDistribution(const std::string & targetDistribution);

    /**
     * Write a human-readable representation of this RepoInfo object
     * into the \a str stream. Useful for logging.
     */
    virtual std::ostream & dumpOn( std::ostream & str ) const;

    /**
     * Write this RepoInfo object into \a str in a <tr>.repo</tt> file format.
     */
    virtual std::ostream & dumpAsIniOn( std::ostream & str ) const;
    /** \deprecated use dumpAsIniOn(ostream) instead */
    ZYPP_DEPRECATED virtual std::ostream & dumpRepoOn( std::ostream & str ) const;

    /**
     * Write an XML representation of this RepoInfo object.
     */
    virtual std::ostream & dumpAsXMLOn( std::ostream & str) const;

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

  typedef shared_ptr<RepoInfo> RepoInfo_Ptr;

  typedef std::list<RepoInfo> RepoInfoList;

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP2_REPOSITORYINFO_H
