/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/repository/RepositoryInfo.cc
 *
*/

#include <string>
#include <iostream>
#include "zypp2/RepositoryInfo.h"

using namespace boost;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  RepositoryInfo::RepositoryInfo()
  : _enabled (indeterminate)
  , _autorefresh(indeterminate)
  , _base_repository( indeterminate )
  {

  }

  RepositoryInfo::RepositoryInfo( const Url & url,
                                  const Pathname & path,
                                  const std::string & alias,
                                  const Pathname & cache_dir,
                                  tribool autorefresh)
  : _enabled (true),
    _autorefresh(autorefresh),
    _base_repository( indeterminate ),
    _url(url),
    _cache_dir(cache_dir),
    _path(path),
    _alias(alias)
  {

  }

  RepositoryInfo & RepositoryInfo::setEnabled( boost::tribool enabled )
  {
    _enabled = enabled;
    return *this;
  }

  RepositoryInfo & RepositoryInfo::setAutorefresh( boost::tribool autorefresh )
  {
    _autorefresh = autorefresh;
    return *this;
  }

  RepositoryInfo & RepositoryInfo::setBaseRepository( bool val_r )
  {
    _base_repository = val_r;
    return *this;
  }

  RepositoryInfo & RepositoryInfo::setUrl( const Url &url )
  {
    _url = url;
    return *this;
  }

  RepositoryInfo & RepositoryInfo::setPath( const Pathname &p )
  {
    _path = p;
    return *this;
  }

  RepositoryInfo & RepositoryInfo::setAlias( const std::string &alias )
  {
    _alias = alias;
    return *this;
  }

  RepositoryInfo & RepositoryInfo::setType( const std::string &t )
  {
    _type = t;
    return *this;
  }

  RepositoryInfo & RepositoryInfo::setCacheDir( const Pathname &p )
  {
    _cache_dir = p;
    return *this;
  }

  RepositoryInfo & RepositoryInfo::setDescription( const std::string &description )
  {
    _description = description;
    return *this;
  }

  RepositoryInfo & RepositoryInfo::setChecksum( const CheckSum &checksum )
  {
    _checksum = checksum;
    return *this;
  }

  RepositoryInfo & RepositoryInfo::setTimestamp( const Date &timestamp )
  {
    _timestamp = timestamp;
    return *this;
  }

  tribool RepositoryInfo::enabled() const
  { return _enabled; }

  tribool RepositoryInfo::autorefresh() const
  { return _autorefresh; }

  boost::tribool RepositoryInfo::baseRepository() const
  { return _base_repository; }

  Pathname RepositoryInfo::cacheDir() const
  { return _cache_dir; }

  Pathname RepositoryInfo::path() const
  { return _path; }

  std::string RepositoryInfo::alias() const
  { return _alias; }

  std::string RepositoryInfo::description() const
  { return _description; }

  CheckSum RepositoryInfo::checksum() const
  { return _checksum; }

  Date RepositoryInfo::timestamp() const
  { return _timestamp; }

  std::string RepositoryInfo::type() const
  { return _type; }

  Url RepositoryInfo::url() const
  { return _url; }

  std::ostream & RepositoryInfo::dumpOn( std::ostream & str ) const
  {
    str << "--------------------------------------" << std::endl;
    str << "- alias       : " << alias() << std::endl;
    str << "- url         : " << url() << std::endl;
    str << "- type        : " << type() << std::endl;
    str << "- baserepository  : " << baseRepository() << std::endl;
    str << "- enabled     : " << enabled() << std::endl;
    str << "- autorefresh : " << autorefresh() << std::endl;
    str << "- path        : " << path() << std::endl;
    str << "- cache_dir   : " << cacheDir() << std::endl;
    return str;
  }
  
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

