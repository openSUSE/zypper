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
    SourceInfo & setUrl( const Url &url );
    SourceInfo & setPath( const Pathname &p );
    SourceInfo & setAlias( const std::string &alias );
    SourceInfo & setType( const std::string &t );
    SourceInfo & setCacheDir( const Pathname &p );
    boost::tribool enabled() const;
    boost::tribool autorefresh() const;
    Pathname cacheDir() const;
    Pathname path() const;
    std::string alias() const;
    std::string type() const;
    Url url() const;
    
    /** Overload to realize stream output. */
    std::ostream & dumpOn( std::ostream & str ) const;
    
    private:
    
    boost::tribool _enabled;
    boost::tribool _autorefresh;
    std::string _type;
    Url _url;
    Pathname _cache_dir;
    Pathname _path;
    std::string _alias;
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
