/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp2/RepoInfo.h
 *
*/
#ifndef ZYPP2_REPOSITORYINFO_H
#define ZYPP2_REPOSITORYINFO_H

#include <iosfwd>
#include <list>
#include <set>
#include "zypp/base/PtrTypes.h"

#include <boost/logic/tribool.hpp>
#include "zypp/Pathname.h"
#include "zypp/Url.h"
#include "zypp/CheckSum.h"
#include "zypp/Date.h"

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
   * \endcode
   *
   * \note A Repository info is a hint about how
   * to create a repository.
   */
  class RepoInfo
  {
    friend std::ostream & operator<<( std::ostream & str, const RepoInfo & obj );
    
    public:
    RepoInfo();
    ~RepoInfo();
    //RepoInfo( const Url & url, const Pathname & path, const std::string & alias = "", boost::tribool autorefresh = boost::indeterminate );
    
    /**
     * unique identifier for this source. If not specified
     * It should be generated from the base url.
     *
     * Normally, in a .repo file the section name is used
     * ( [somerepo] )
     */
    std::string alias() const;
    
    /**
     * The base Url is the Url of the repository that generates
     * the authoritative metadata this repository provides.
     *
     * For example for the url http://updates.novell.com/10.2
     * the base url is http://updates.novell.com/10.2.
     * For the url http://host.com/mirror/update.novell.com/10.2
     * the base url is http://updates.novell.com/10.2
     *
     * This can't be empty in order the repository to be valid
     */
    Url baseUrl() const;

    /**
     * Urls is a list of Urls where this repository
     * is located.
     * If empty, the base url will be used.
     */
    std::set<Url> urls() const;
    
    typedef std::set<Url>::const_iterator urls_const_iterator;
    
    /**
     * iterator that points at begin of repository urls
     */
    urls_const_iterator urlsBegin() const;
    
    /**
     * iterator that points at end of repository urls
     */
    urls_const_iterator urlsEnd() const;
    
    /**
     * Path on the url where the repository root
     * is located.
     */
    Pathname path() const;
    
    /**
     * If enabled is false, then this repository must be ignored as if does
     * not exists, except when checking for duplicate alias.
     */
    boost::tribool enabled() const;
    
    /**
     * If true, the repostory must be refreshed before creating resolvables
     * from it
     */
    boost::tribool autorefresh() const;
    
    /**
     * Type of repository,
     * FIXME should be an enum?
     */
    std::string type() const;
    
    /**
     * \short Repository short label
     *
     * Short label or description of the repository, to be used on
     * the user interface.
     * ie: "SUSE Linux 10.2 updates"
     */
    std::string name() const;
    
    /**
     * Checksum of the repository.
     * Usually the checksum of the index, but any
     * checksum that changes when the repository changes
     * in any way is sufficient.
     */
    CheckSum checksum() const;
    
    /**
     * timestamp of the repository. If the repository
     * changes, it has to be updated as well with the
     * new timestamp.
     */
    Date timestamp() const;
    
    /**
     * Set the base url. \see baseUrl
     * \param url The base url for the repository.
     */
    RepoInfo & setBaseUrl( const Url &url );
    
    /**
     * enable or disable the repository \see enabled
     * \param enabled
     */
    RepoInfo & setEnabled( boost::tribool enabled );
    
    /**
     * enable or disable autorefresh \see autorefresh
     * \param enabled
     */
    RepoInfo & setAutorefresh( boost::tribool autorefresh );
    
    /**
     * set the repository path \see path
     * \param p
     */
    RepoInfo & setPath( const Pathname &p );
    
    /**
     * set the repository alias \see alias
     * \param alias
     */
    RepoInfo & setAlias( const std::string &alias );
    
    /**
     * set the repository type \see type
     * \param t
     */
    RepoInfo & setType( const std::string &t );
    
    /**
     * set the repository name \see name
     * \param name
     */
    RepoInfo & setName( const std::string &name );
    
    /**
     * set the repository checksum \see checksum
     * \param checksum
     */
    RepoInfo & setChecksum( const CheckSum &checksum );
    
    /**
     * set the repository timestamp \see timestamp
     * \param timestamp
     */
    RepoInfo & setTimestamp( const Date &timestamp );
    
    std::ostream & dumpOn( std::ostream & str ) const;
    
    class Impl;
  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoInfo Stream output */
  std::ostream & operator<<( std::ostream & str, const RepoInfo & obj );

  typedef std::list<RepoInfo> RepoInfoList;
  
  /////////////////////////////////////////////////////////////////
} // namespace zypp2
///////////////////////////////////////////////////////////////////
#endif // ZYPP2_REPOSITORYINFO_H
