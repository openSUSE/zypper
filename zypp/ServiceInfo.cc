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

#include "zypp/base/String.h"
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
    typedef ServiceInfo::CatalogsToDisable CatalogsToDisable;

  public:
    Url url;
    repo::ServiceType type;
    CatalogsToEnable  catalogsToEnable;
    CatalogsToDisable catalogsToDisable;

  public:
    Impl() : repo::RepoInfoBase::Impl() {}

    Impl(const Url & url_) : url(url_) {}
    
    void setProbedType( const repo::ServiceType & t ) const
    {
      if ( type == repo::ServiceType::NONE
           && t != repo::ServiceType::NONE )
      {
        // lazy init!
        const_cast<Impl*>(this)->type = t;
      }
    }

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

  repo::ServiceType ServiceInfo::type() const
  { return _pimpl->type; }
  void ServiceInfo::setType( const repo::ServiceType & type )
  { _pimpl->type = type; }

  void ServiceInfo::setProbedType( const repo::ServiceType &t ) const
  { _pimpl->setProbedType( t ); }

  bool ServiceInfo::catalogsToEnableEmpty() const
  { return _pimpl->catalogsToEnable.empty(); }

  ServiceInfo::CatalogsToEnable::size_type ServiceInfo::catalogsToEnableSize() const
  { return _pimpl->catalogsToEnable.size(); }

  ServiceInfo::CatalogsToEnable::const_iterator ServiceInfo::catalogsToEnableBegin() const
  { return _pimpl->catalogsToEnable.begin(); }

  ServiceInfo::CatalogsToEnable::const_iterator ServiceInfo::catalogsToEnableEnd() const
  { return _pimpl->catalogsToEnable.end(); }

  bool ServiceInfo::catalogToEnableFind( const std::string & alias_r ) const
  { return( _pimpl->catalogsToEnable.find( alias_r ) != _pimpl->catalogsToEnable.end() ); }

  void ServiceInfo::addCatalogToEnable( const std::string & alias_r )
  { _pimpl->catalogsToEnable.insert( alias_r ); }

  void ServiceInfo::delCatalogToEnable( const std::string & alias_r )
  { _pimpl->catalogsToEnable.erase( alias_r ); }


  bool ServiceInfo::catalogsToDisableEmpty() const
  { return _pimpl->catalogsToDisable.empty(); }

  ServiceInfo::CatalogsToDisable::size_type ServiceInfo::catalogsToDisableSize() const
  { return _pimpl->catalogsToDisable.size(); }

  ServiceInfo::CatalogsToDisable::const_iterator ServiceInfo::catalogsToDisableBegin() const
  { return _pimpl->catalogsToDisable.begin(); }

  ServiceInfo::CatalogsToDisable::const_iterator ServiceInfo::catalogsToDisableEnd() const
  { return _pimpl->catalogsToDisable.end(); }

  bool ServiceInfo::catalogToDisableFind( const std::string & alias_r ) const
  { return( _pimpl->catalogsToDisable.find( alias_r ) != _pimpl->catalogsToDisable.end() ); }

  void ServiceInfo::addCatalogToDisable( const std::string & alias_r )
  { _pimpl->catalogsToDisable.insert( alias_r ); }

  void ServiceInfo::delCatalogToDisable( const std::string & alias_r )
  { _pimpl->catalogsToDisable.erase( alias_r ); }


  std::ostream & ServiceInfo::dumpAsIniOn( std::ostream & str ) const
  {
    RepoInfoBase::dumpAsIniOn(str)
      << "url = " << url() << endl
      << "type = " << type() << endl;

    if ( ! catalogsToEnableEmpty() )
      str << "catalogstoenable = " << str::joinEscaped( catalogsToEnableBegin(), catalogsToEnableEnd() ) << endl;
    if ( ! catalogsToDisableEmpty() )
      str << "catalogstodisable = " << str::joinEscaped( catalogsToDisableBegin(), catalogsToDisableEnd() ) << endl;
    return str;
  }

  std::ostream & ServiceInfo::dumpAsXMLOn(std::ostream & str) const
  { return dumpAsXMLOn(str, ""); }

  ostream & ServiceInfo::dumpAsXMLOn( ostream & str, const string & content) const
  {
    str
      << "<service"
      << " alias=\"" << escape(alias()) << "\""
      << " name=\"" << escape(name()) << "\""
      << " enabled=\"" << enabled() << "\""
      << " autorefresh=\"" << autorefresh() << "\""
      << " url=\"" << escape(url().asString()) << "\""
      << " type=\"" << type().asString() << "\"";

    if (content.empty())
      str << "/>" << endl;
    else
      str << ">" << endl << content << "</service>" << endl;

    return str;
  }


  std::ostream & operator<<( std::ostream& str, const ServiceInfo &obj )
  {
    return obj.dumpAsIniOn(str);
  }


///////////////////////////////////////////////////////////////////////////////
} //namespace zypp
///////////////////////////////////////////////////////////////////////////////
