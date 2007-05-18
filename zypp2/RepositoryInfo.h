/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_RepositoryInfo_H
#define ZYPP_RepositoryInfo_H

#include <list>
#include <set>
#include <boost/logic/tribool.hpp>
#include "zypp/Pathname.h"
#include "zypp/Url.h"
#include "zypp/CheckSum.h"
#include "zypp/Date.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /**
   * The class RepositoryInfo represents everything that
   * is known about a software repository.
   */
  class RepositoryInfo
  {
    public:

    RepositoryInfo();

    RepositoryInfo( const Url & url, const Pathname & path, const std::string & alias = "", boost::tribool autorefresh = boost::indeterminate );
    
    /**
     * unique identifier for this source. If not specified
     * It should be generated from the base url.
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
     * Description of the repository, to be used on
     * the user interface.
     */
    std::string description() const;
    
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
    RepositoryInfo & setBaseUrl( const Url &url );
    
    /**
     * enable or disable the repository \see enabled
     * \param enabled
     */
    RepositoryInfo & setEnabled( boost::tribool enabled );
    
    /**
     * enable or disable autorefresh \see autorefresh
     * \param enabled
     */
    RepositoryInfo & setAutorefresh( boost::tribool autorefresh );
    
    /**
     * set the repository path \see path
     * \param p
     */
    RepositoryInfo & setPath( const Pathname &p );
    
    /**
     * set the repository alias \see alias
     * \param alias
     */
    RepositoryInfo & setAlias( const std::string &alias );
    
    /**
     * set the repository type \see type
     * \param t
     */
    RepositoryInfo & setType( const std::string &t );
    
    /**
     * set the repository description \see description
     * \param description
     */
    RepositoryInfo & setDescription( const std::string &description );
    
    /**
     * set the repository checksum \see checksum
     * \param checksum
     */
    RepositoryInfo & setChecksum( const CheckSum &checksum );
    
    /**
     * set the repository timestamp \see timestamp
     * \param timestamp
     */
    RepositoryInfo & setTimestamp( const Date &timestamp );
    
    /** Overload to realize stream output. */
    std::ostream & dumpOn( std::ostream & str ) const;
    
    private:

    boost::tribool _enabled;
    boost::tribool _autorefresh;
    std::string _type;
    Url _baseurl;
    std::set<Url> _urls;
    Pathname _path;
    std::string _alias;
    std::string _description;
    CheckSum _checksum;
    Date _timestamp;
  };

  /** \relates RepositoryInfo Stream output */
  inline std::ostream & operator<<( std::ostream & str, const RepositoryInfo & obj )
  { return obj.dumpOn( str ); }

  typedef std::list<RepositoryInfo> RepositoryInfoList;

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RepositoryInfo_H


