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
  {

  }

  RepositoryInfo::RepositoryInfo( const Url & url,
                                  const Pathname & path,
                                  const std::string & alias,
                                  tribool autorefresh)
  : _enabled (true),
    _autorefresh(autorefresh),
    _baseurl(url),
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

  RepositoryInfo & RepositoryInfo::setBaseUrl( const Url &url )
  {
    _baseurl = url;
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

  Url RepositoryInfo::baseUrl() const
  { return _baseurl; }

  std::set<Url> RepositoryInfo::urls() const
  { return _urls; }
    
  RepositoryInfo::urls_const_iterator RepositoryInfo::urlsBegin() const
  { return _urls.begin(); }
    
  RepositoryInfo::urls_const_iterator RepositoryInfo::urlsEnd() const
  { return _urls.end(); }
  
  std::ostream & RepositoryInfo::dumpOn( std::ostream & str ) const
  {
    str << "--------------------------------------" << std::endl;
    str << "- alias       : " << alias() << std::endl;
    str << "- url         : " << baseUrl() << std::endl;
    str << "- type        : " << type() << std::endl;
    str << "- enabled     : " << enabled() << std::endl;
    str << "- autorefresh : " << autorefresh() << std::endl;
    str << "- path        : " << path() << std::endl;
    return str;
  }
  
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

