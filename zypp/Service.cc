#include "zypp/Service.h"

#include <ostream>

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
      RepoContainer repos;
      string name;
      Url url;
      Pathname loc;
      Impl() : name("") {};
      Impl(const string& name_) : name(name_) {};
      Impl(const string& name_, const Url& url_) : name(name_), url(url_) {};
  };

  Service::Service() : _pimpl( new Impl() ) {};

  Service::Service(const string& name) : _pimpl( new Impl(name) ) {};
  Service::Service(const string& name, const Url& url)
    : _pimpl( new Impl(name,url) ) {};

  const Service Service::noService;

  string Service::name() const { return _pimpl->name; }
  Url Service::url() const { return _pimpl->url; }

  void Service::setUrl( const Url& url ) { _pimpl->url = url; }

  bool Service::empty() const { return _pimpl->repos.empty(); }
  Service::size_type Service::size() const { return _pimpl->repos.size(); }
  Service::const_iterator Service::begin() const { return _pimpl->repos.begin(); }
  Service::const_iterator Service::end() const { return _pimpl->repos.end(); }

  void Service::addRepo( const std::string& alias )
  {
    _pimpl->repos.push_back(alias);
  }

  void Service::dumpServiceOn( std::ostream& str ) const
  {
    str << endl;
    str << "[" << name() << "]" << endl;
    str << "url = " << url() << endl;
    for_( it, begin(), end() )
    {
      str << "alias = " << *it << endl;
    }
    str << endl;
  }

  void Service::refresh( RepoManager& repomanager ) const
  {
    //download index file
    media::MediaManager mediamanager;
    media::MediaAccessId mid = mediamanager.open( url() );
    mediamanager.provideFile( mid, "repo/repoindex.xml" );
    Pathname path = mediamanager.localPath(mid, "repo/repoindex.xml" );

    //parse it
    RepoInfoCollector collector;
    parser::RepoindexFileReader reader( path,
      bind( &RepoInfoCollector::collect, &collector, _1 ) );

    // set base url for all collected repositories
    for_( it, collector.repos.begin(), collector.repos.end())
    {
      it->setBaseUrl( url() );
    }

    //compare old and new repositories (hope not to much, if it change
    // then construct set and use set operation on it)

    std::list<RepoInfo> krepos = repomanager.knownRepositories();

    //find old to remove
    for_( it, begin(), end() )
    {
      bool found = false;

      for_( it2, collector.repos.begin(), collector.repos.end() )
        if ( *it == it2->alias() )
        {
          found = true;
          break;
        }

      if( !found )
        repomanager.removeRepository( repomanager.getRepositoryInfo( *it ) );
    }

    //find new to add
    for_( it, collector.repos.begin(), collector.repos.end() )
    {
      bool found = false;

      for_( it2, begin(), end() )
        if( it->alias() == *it2 )
        {
          found = true;
          break;
        }

      if (!found)
        repomanager.addRepository( *it );
    }

    //update internal alias storage
    _pimpl->repos.clear();
    for_( it, collector.repos.begin(), collector.repos.end() )
    {
      _pimpl->repos.push_back(it->alias());
    }
    
    mediamanager.close( mid );
  }

  Pathname Service::location() const
  {
    return _pimpl->loc;
  }

  void Service::setLocation( const Pathname& location ) const
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
