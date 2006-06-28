/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/SourceInfo.cc
 *
*/

#include <string>
#include <iostream>
#include "zypp/source/SourceInfo.h"

using namespace boost;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace source
{
  
  SourceInfo::SourceInfo() :
      _enabled (indeterminate),
  _autorefresh(indeterminate)
  {
      
  }
    
  SourceInfo::SourceInfo( const Url & url, const Pathname & path, const std::string & alias, const Pathname & cache_dir, tribool autorefresh)
  : _enabled (true),
  _autorefresh(autorefresh),
  _url(url),
  _cache_dir(cache_dir),
  _path(path),
  _alias(alias)
  {
      
  }
    
  SourceInfo & SourceInfo::setEnabled( boost::tribool enabled )
  {
    _enabled = enabled;
    return *this;
  }
    
  SourceInfo & SourceInfo::setAutorefresh( boost::tribool autorefresh )
  {
    _autorefresh = autorefresh;
    return *this;
  }
    
  SourceInfo & SourceInfo::setUrl( const Url &url )
  {
    _url = url;
    return *this;
  }
    
  SourceInfo & SourceInfo::setPath( const Pathname &p )
  {
    _path = p;
    return *this;
  }
    
  SourceInfo & SourceInfo::setAlias( const std::string &alias )
  {
    _alias = alias;
    return *this;
  }
    
  SourceInfo & SourceInfo::setType( const std::string &t )
  {
    _type = t;
    return *this;
  }
    
  SourceInfo & SourceInfo::setCacheDir( const Pathname &p )
  {
    _cache_dir = p;
    return *this;
  }
     
  tribool SourceInfo::enabled() const
  { return _enabled; }
        
  tribool SourceInfo::autorefresh() const
  { return _enabled; }    
    
  Pathname SourceInfo::cacheDir() const
  { return _cache_dir; }
    
  Pathname SourceInfo::path() const
  { return _path; }
    
  std::string SourceInfo::alias() const
  { return _alias; }
    
  std::string SourceInfo::type() const
  { return _type; }
    
  Url SourceInfo::url() const
  { return _url; }
  
  std::ostream & SourceInfo::dumpOn( std::ostream & str ) const
  {
    str << "--------------------------------------" << std::endl;
    str << "- alias       : " << alias() << std::endl;
    str << "- url         : " << url() << std::endl;
    str << "- type        : " << type() << std::endl;
    str << "- enabled     : " << enabled() << std::endl;
    str << "- autorefresh : " << autorefresh() << std::endl;
    str << "- path        : " << path() << std::endl;
    str << "- cache_dir   : " << cacheDir() << std::endl;
    return str;
  }
  
}
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
