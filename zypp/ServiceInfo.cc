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

#include "zypp/parser/xml/XmlEscape.h"

#include "zypp/RepoInfo.h"
#include "zypp/parser/RepoindexFileReader.h"
#include "zypp/repo/RepoInfoBaseImpl.h"

#include "zypp/ServiceInfo.h"

using namespace std;
using zypp::xml::escape;

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
    typedef ServiceInfo::CatalogsToEnable CatalogsToEnable;

  public:
    Url url;
    CatalogsToEnable catalogsToEnable;

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


  bool catalogsToEnableEmpty() const
  { return _pimpl->catalogsToEnable.empty(); }

  CatalogsToEnable::size_type catalogsToEnableSize() const
  { return _pimpl->catalogsToEnable.size(); }

  CatalogsToEnable::const_iterator catalogsToEnableBegin() const
  { return _pimpl->catalogsToEnable.begin(); }

  CatalogsToEnable::const_iterator catalogsToEnableEnd() const
  { return _pimpl->catalogsToEnable.end(); }

  bool catalogToEnableFind( const std::string & alias_r ) const
  { return( _pimpl->catalogsToEnable.find( alias_r ) != _pimpl->catalogsToEnable.end() ); }

  void addCatalogToEnable( const std::string & alias_r )
  { return _pimpl->catalogsToEnable.insert( alias_r ); }

  void delCatalogToEnable( const std::string & alias_r )
  { return _pimpl->catalogsToEnable.erase( alias_r ); }


  std::ostream & ServiceInfo::dumpAsIniOn( std::ostream & str ) const
  {
    return RepoInfoBase::dumpAsIniOn(str)
        << "url = " << url() << endl;
  }

  std::ostream & ServiceInfo::dumpAsXMLOn( std::ostream & str) const
  {
    return str
      << "<service"
      << " alias=\"" << escape(alias()) << "\""
      << " name=\"" << escape(name()) << "\""
      << " enabled=\"" << enabled() << "\""
      << " autorefresh=\"" << autorefresh() << "\""
      << " url=\"" << escape(url().asString()) << "\""
      << "/>" << endl;
  }

  std::ostream & operator<<( std::ostream& str, const ServiceInfo &obj )
  {
    return obj.dumpAsIniOn(str);
  }


///////////////////////////////////////////////////////////////////////////////
} //namespace zypp
///////////////////////////////////////////////////////////////////////////////
