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

  SourceInfo::SourceInfo()
  : _enabled (indeterminate)
  , _autorefresh(indeterminate)
  , _baseSource( false )
  {

  }

  SourceInfo::SourceInfo( const Url & url, const Pathname & path, const std::string & alias, const Pathname & cache_dir, tribool autorefresh)
  : _enabled (true),
  _autorefresh(autorefresh),
  _baseSource( false ),
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

  SourceInfo & SourceInfo::setBaseSource( bool val_r )
  {
    _baseSource = val_r;
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

  SourceInfo & SourceInfo::setDescription( const std::string &description )
  {
    _description = description;
    return *this;
  }

  SourceInfo & SourceInfo::setChecksum( const CheckSum &checksum )
  {
    _checksum = checksum;
    return *this;
  }

  SourceInfo & SourceInfo::setTimestamp( const Date &timestamp )
  {
    _timestamp = timestamp;
    return *this;
  }

  tribool SourceInfo::enabled() const
  { return _enabled; }

  tribool SourceInfo::autorefresh() const
  { return _enabled; }

  bool SourceInfo::baseSource() const
  { return _baseSource; }

  Pathname SourceInfo::cacheDir() const
  { return _cache_dir; }

  Pathname SourceInfo::path() const
  { return _path; }

  std::string SourceInfo::alias() const
  { return _alias; }

  std::string SourceInfo::description() const
  { return _description; }

  CheckSum SourceInfo::checksum() const
  { return _checksum; }

  Date SourceInfo::timestamp() const
  { return _timestamp; }

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
    str << "- basesource  : " << baseSource() << std::endl;
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
