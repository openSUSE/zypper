/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file       zypp/ServiceInfo.cc
 *
 */
#include <ostream>
#include <iostream>

#include "zypp/RepoInfo.h"
#include "zypp/parser/RepoindexFileReader.h"
#include "zypp/repo/RepoInfoBaseImpl.h"

#include "zypp/ServiceInfo.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
namespace zypp 
{//////////////////////////////////////////////////////////////////////////////


  struct RepoInfoCollector
  {
    vector<RepoInfo> repos;
    bool collect(const RepoInfo & info)
    {
      repos.push_back(info);
      return true;
    }
  };

  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : ServiceInfo::Impl
  //
  struct ServiceInfo::Impl : public repo::RepoInfoBase::Impl
  {
  public:
    Url url;

  public:
    Impl() : repo::RepoInfoBase::Impl() {}

    Impl(const Url & url_) : url(url_) {}

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );

    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////


  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : ServiceInfo::Impl
  //
  ///////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(ServiceInfo);

  const ServiceInfo ServiceInfo::noService;
  
  ServiceInfo::ServiceInfo() : _pimpl( new Impl() ) {}

  ServiceInfo::ServiceInfo(const string & alias)
    : repo::RepoInfoBase(alias), _pimpl( new Impl() )
  {}

  ServiceInfo::ServiceInfo(const string & alias, const Url & url)
    : repo::RepoInfoBase(alias), _pimpl( new Impl(url) )
  {}

  Url ServiceInfo::url() const { return _pimpl->url; }
  void ServiceInfo::setUrl( const Url& url ) { _pimpl->url = url; }


  std::ostream & ServiceInfo::dumpAsIniOn( std::ostream & str ) const
  {
    return RepoInfoBase::dumpAsIniOn(str) << "url = " << url() << endl;
  }

  std::ostream & operator<<( std::ostream& str, const ServiceInfo &obj )
  {
    return obj.dumpAsIniOn(str);
  }


///////////////////////////////////////////////////////////////////////////////
} //namespace zypp
///////////////////////////////////////////////////////////////////////////////
