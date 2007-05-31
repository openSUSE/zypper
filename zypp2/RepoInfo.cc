/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/RepoInfo.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp2/RepoInfo.h"

using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoInfo::Impl
  //
  /** RepoInfo implementation. */
  struct RepoInfo::Impl
  {
    
    Impl()
      : enabled (indeterminate),
        autorefresh(indeterminate)
    {}
  public:
    boost::tribool enabled;
    boost::tribool autorefresh;
    std::string type;
    Url baseurl;
    std::set<Url> urls;
    Pathname path;
    std::string alias;
    std::string name;
    CheckSum checksum;
    Date timestamp;

  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl );
      return _nullimpl;
    }

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoInfo::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const RepoInfo::Impl & obj )
  {
    return str << "RepoInfo::Impl";
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoInfo
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : RepoInfo::RepoInfo
  //	METHOD TYPE : Ctor
  //
  RepoInfo::RepoInfo()
  : _pimpl( Impl::nullimpl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : RepoInfo::~RepoInfo
  //	METHOD TYPE : Dtor
  //
  RepoInfo::~RepoInfo()
  {}

  
  
  RepoInfo & RepoInfo::setEnabled( boost::tribool enabled )
  {
    _pimpl->enabled = enabled;
    return *this;
  }

  RepoInfo & RepoInfo::setAutorefresh( boost::tribool autorefresh )
  {
    _pimpl->autorefresh = autorefresh;
    return *this;
  }

  RepoInfo & RepoInfo::setBaseUrl( const Url &url )
  {
    _pimpl->baseurl = url;
    return *this;
  }

  RepoInfo & RepoInfo::setPath( const Pathname &p )
  {
    _pimpl->path = p;
    return *this;
  }

  RepoInfo & RepoInfo::setAlias( const std::string &alias )
  {
    _pimpl->alias = alias;
    return *this;
  }

  RepoInfo & RepoInfo::setType( const std::string &t )
  {
    _pimpl->type = t;
    return *this;
  }

  RepoInfo & RepoInfo::setName( const std::string &name )
  {
    _pimpl->name = name;
    return *this;
  }

  RepoInfo & RepoInfo::setChecksum( const CheckSum &checksum )
  {
    _pimpl->checksum = checksum;
    return *this;
  }

  RepoInfo & RepoInfo::setTimestamp( const Date &timestamp )
  {
    _pimpl->timestamp = timestamp;
    return *this;
  }

  tribool RepoInfo::enabled() const
  { return _pimpl->enabled; }

  tribool RepoInfo::autorefresh() const
  { return _pimpl->autorefresh; }

  
  Pathname RepoInfo::path() const
  { return _pimpl->path; }

  std::string RepoInfo::alias() const
  { return _pimpl->alias; }

  std::string RepoInfo::name() const
  { return _pimpl->name; }

  CheckSum RepoInfo::checksum() const
  { return _pimpl->checksum; }

  Date RepoInfo::timestamp() const
  { return _pimpl->timestamp; }

  std::string RepoInfo::type() const
  { return _pimpl->type; }

  Url RepoInfo::baseUrl() const
  { return _pimpl->baseurl; }

  std::set<Url> RepoInfo::urls() const
  { return _pimpl->urls; }
    
  RepoInfo::urls_const_iterator RepoInfo::urlsBegin() const
  { return _pimpl->urls.begin(); }
    
  RepoInfo::urls_const_iterator RepoInfo::urlsEnd() const
  { return _pimpl->urls.end(); }
  
  std::ostream & RepoInfo::dumpOn( std::ostream & str ) const
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
} // namespace zypp2
///////////////////////////////////////////////////////////////////
