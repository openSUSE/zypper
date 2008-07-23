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

  struct RepoInfoCollector {
    std::vector<RepoInfo> repos;
    bool collect(const RepoInfo& info)
    {
      repos.push_back(info);
      return true;
    }
  };

  class Service::Impl{
    public:
      string name;
      Url url;
      DefaultIntegral<bool,false> enabled;
      Pathname loc;

      Impl() : name("") {};
      Impl(const string& name_) : name(name_) {};
      Impl(const string& name_, const Url& url_) : name(name_), url(url_) {};
    private:
      friend Impl * rwcowClone<Impl>( const Impl * rhs );
      /** clone for RWCOW_pointer */
      Impl * clone() const
      { return new Impl( *this ); }
  };

  Service::Service() : _pimpl( new Impl() ) {};

  Service::Service(const string& name) : _pimpl( new Impl(name) ) {};
  Service::Service(const string& name, const Url& url)
    : _pimpl( new Impl(name,url) ) {};

  const Service Service::noService;

  string Service::name() const { return _pimpl->name; }
  void Service::setName( const std::string& name ) { _pimpl->name = name; }

  Url Service::url() const { return _pimpl->url; }
  void Service::setUrl( const Url& url ) { _pimpl->url = url; }

  bool Service::enabled() const { return _pimpl->enabled; }
  void Service::setEnabled( const bool enabled ) { _pimpl->enabled = enabled; }  

  void Service::dumpServiceOn( std::ostream& str ) const
  {
    str << endl;
    str << "[" << name() << "]" << endl;
    str << "url = " << url() << endl;
    str << "enabled = " << ( enabled() ? "1" : "0") << endl;
    str << endl;
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
