/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file       zypp/Service.cc
 *
 */
#include "zypp/Service.h"

#include <ostream>

#include "zypp/base/DefaultIntegral.h"
#include "zypp/Url.h"
#include "zypp/RepoInfo.h"
#include "zypp/media/MediaManager.h"
#include "zypp/parser/RepoindexFileReader.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
namespace zypp 
{//////////////////////////////////////////////////////////////////////////////


  struct RepoInfoCollector
  {
    vector<RepoInfo> repos;
    bool collect(const RepoInfo& info)
    {
      repos.push_back(info);
      return true;
    }
  };


  class Service::Impl
  {
  public:
    string alias;
    string name;
    Url url;
    DefaultIntegral<bool,false> enabled;
    Pathname loc;

    Impl() {}
    Impl(const string & alias_) : alias(alias_) {}
    Impl(const string & alias_, const Url& url_) : alias(alias_), url(url_) {}

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );

    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };


  const Service Service::noService;

  Service::Service() : _pimpl( new Impl() ) {}

  Service::Service(const string & alias) : _pimpl( new Impl(alias) ) {}
  Service::Service(const string & alias, const Url & url)
    : _pimpl( new Impl(alias, url) ) {}


  string Service::alias() const { return _pimpl->alias; }
  void Service::setAlias( const std::string & alias ) { _pimpl->alias = alias; }

  string Service::name() const
  { if (_pimpl->name.empty()) return _pimpl->alias; return _pimpl->name; }
  void Service::setName( const std::string& name ) { _pimpl->name = name; }

  Url Service::url() const { return _pimpl->url; }
  void Service::setUrl( const Url& url ) { _pimpl->url = url; }

  bool Service::enabled() const { return _pimpl->enabled; }
  void Service::setEnabled( const bool enabled ) { _pimpl->enabled = enabled; }  

  void Service::dumpServiceOn( std::ostream& str ) const
  {
    str << "[" << alias() << "]" << endl;
    str << "name = " << name() << endl;
    str << "url = " << url() << endl;
    str << "enabled = " << ( enabled() ? "1" : "0") << endl;
  }

  Pathname Service::location() const
  {
    return _pimpl->loc;
  }

  void Service::setLocation( const Pathname& location )
  {
    _pimpl->loc = location;
  }

  std::ostream & operator<<( std::ostream& str, const Service &obj )
  {
    obj.dumpServiceOn(str);
    return str;
  }


///////////////////////////////////////////////////////////////////////////////
} //namespace zypp
///////////////////////////////////////////////////////////////////////////////
