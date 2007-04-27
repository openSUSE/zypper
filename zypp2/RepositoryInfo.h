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

#include <boost/logic/tribool.hpp>
#include "zypp/Pathname.h"
#include "zypp/Url.h"
#include "zypp/CheckSum.h"
#include "zypp/Date.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class RepositoryInfo
  {
    public:

    RepositoryInfo();

    RepositoryInfo( const Url & url, const Pathname & path, const std::string & alias = "", const Pathname & cache_dir = "", boost::tribool autorefresh = boost::indeterminate );

    RepositoryInfo & setEnabled( boost::tribool enabled );
    RepositoryInfo & setAutorefresh( boost::tribool autorefresh );
    RepositoryInfo & setBaseRepository( bool val_r );
    RepositoryInfo & setUrl( const Url &url );
    RepositoryInfo & setPath( const Pathname &p );
    RepositoryInfo & setAlias( const std::string &alias );
    RepositoryInfo & setType( const std::string &t );
    RepositoryInfo & setCacheDir( const Pathname &p );
    RepositoryInfo & setDescription( const std::string &description );
    RepositoryInfo & setChecksum( const CheckSum &checksum );
    RepositoryInfo & setTimestamp( const Date &timestamp );
    boost::tribool enabled() const;
    boost::tribool autorefresh() const;
    boost::tribool baseRepository() const;
    Pathname cacheDir() const;
    Pathname path() const;
    std::string alias() const;
    std::string type() const;
    std::string description() const;
    CheckSum checksum() const;
    Date timestamp() const;
    Url url() const;


    /** Overload to realize stream output. */
    std::ostream & dumpOn( std::ostream & str ) const;

    private:

    boost::tribool _enabled;
    boost::tribool _autorefresh;
    boost::tribool _base_repository;
    std::string _type;
    Url _url;
    Pathname _cache_dir;
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


