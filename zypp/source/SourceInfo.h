/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/SourceInfo.h
 *
*/
#ifndef ZYPP_SourceInfo_H
#define ZYPP_SourceInfo_H

#include <list>

#include <boost/logic/tribool.hpp>
#include "zypp/Pathname.h"
#include "zypp/Url.h"
#include "zypp/CheckSum.h"
#include "zypp/Date.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace source
{

  class SourceInfo
  {
    public:

    SourceInfo();

    SourceInfo( const Url & url, const Pathname & path, const std::string & alias = "", const Pathname & cache_dir = "", boost::tribool autorefresh = boost::indeterminate);

    SourceInfo & setEnabled( boost::tribool enabled );
    SourceInfo & setAutorefresh( boost::tribool autorefresh );
    SourceInfo & setBaseSource( bool val_r );
    SourceInfo & setUrl( const Url &url );
    SourceInfo & setPath( const Pathname &p );
    SourceInfo & setAlias( const std::string &alias );
    SourceInfo & setType( const std::string &t );
    SourceInfo & setCacheDir( const Pathname &p );
    SourceInfo & setDescription( const std::string &description );
    SourceInfo & setChecksum( const CheckSum &checksum );
    SourceInfo & setTimestamp( const Date &timestamp );
    boost::tribool enabled() const;
    boost::tribool autorefresh() const;
    bool baseSource() const;
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
    bool _baseSource;
    std::string _type;
    Url _url;
    Pathname _cache_dir;
    Pathname _path;
    std::string _alias;
    std::string _description;
    CheckSum _checksum;
    Date _timestamp;
  };

  /** \relates SourceInfo Stream output */
  inline std::ostream & operator<<( std::ostream & str, const SourceInfo & obj )
  { return obj.dumpOn( str ); }

  typedef std::list<SourceInfo> SourceInfoList;
}
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SourceInfo_H
