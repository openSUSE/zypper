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

using namespace boost;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace source
{
  
  class SourceInfo
  {
    public:
      
    SourceInfo() :
        _enabled (indeterminate),
        _autorefresh(indeterminate)
    {
      
    }
    
    SourceInfo( const Url & url, const Pathname & path, const std::string & alias = "", const Pathname & cache_dir = "", tribool autorefresh = indeterminate)
      : _enabled (true),
    _autorefresh(autorefresh),
    _url(url),
    _cache_dir(cache_dir),
    _path(path),
    _alias(alias)
    {
      
    }
    
    SourceInfo & setEnabled( bool enabled )
    {
      _enabled = enabled;
      return *this;
    }
    
    SourceInfo & setAutorefresh( bool autorefresh )
    {
      _autorefresh = autorefresh;
      return *this;
    }
    
    SourceInfo & setUrl( const Url &url )
    {
      _url = url;
      return *this;
    }
    
    SourceInfo & setPath( const Pathname &p )
    {
      _path = p;
      return *this;
    }
    
    SourceInfo & setAlias( const std::string &alias )
    {
      _alias = alias;
      return *this;
    }
    
    SourceInfo & setType( const std::string &t )
    {
      _type = t;
      return *this;
    }
    
    SourceInfo & setCacheDir( const Pathname &p )
    {
      _cache_dir = p;
      return *this;
    }
     
    tribool enabled() const
    { return _enabled; }
        
    tribool autorefresh() const
    { return _enabled; }    
    
    Pathname cacheDir() const
    { return _cache_dir; }
    
    Pathname path() const
    { return _path; }
    
    std::string alias() const
    { return _alias; }
    
    std::string type() const
    { return _type; }
    
    Url url() const
    { return _url; }
    
    private:
    
    tribool _enabled;
    tribool _autorefresh;
    std::string _type;
    Url _url;
    Pathname _cache_dir;
    Pathname _path;
    std::string _alias;
  };  
  
  typedef std::list<SourceInfo> SourceInfoList;
}
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SourceInfo_H
