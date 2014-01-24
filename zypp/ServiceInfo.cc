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
#include "zypp/repo/RepoInfoBaseImpl.h"

#include "zypp/ServiceInfo.h"

using namespace std;
using zypp::xml::escape;

///////////////////////////////////////////////////////////////////////////////
namespace zypp
{//////////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : ServiceInfo::Impl
  //
  struct ServiceInfo::Impl : public repo::RepoInfoBase::Impl
  {
    typedef ServiceInfo::ReposToEnable  ReposToEnable;
    typedef ServiceInfo::ReposToDisable ReposToDisable;

  public:
    Url url;
    repo::ServiceType type;
    ReposToEnable  reposToEnable;
    ReposToDisable reposToDisable;

  public:
    Impl()
      : repo::RepoInfoBase::Impl()
      , type(repo::ServiceType::NONE_e)
    {}

    Impl(const Url & url_)
      : repo::RepoInfoBase::Impl()
      , url(url_)
      , type(repo::ServiceType::NONE_e)
    {}

    ~Impl()
    {}

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

  ServiceInfo::~ServiceInfo()
  {}

  Url ServiceInfo::url() const { return _pimpl->url; }
  void ServiceInfo::setUrl( const Url& url ) { _pimpl->url = url; }

  repo::ServiceType ServiceInfo::type() const
  { return _pimpl->type; }
  void ServiceInfo::setType( const repo::ServiceType & type )
  { _pimpl->type = type; }

  void ServiceInfo::setProbedType( const repo::ServiceType &t ) const
  { _pimpl->setProbedType( t ); }

  bool ServiceInfo::reposToEnableEmpty() const
  { return _pimpl->reposToEnable.empty(); }

  ServiceInfo::ReposToEnable::size_type ServiceInfo::reposToEnableSize() const
  { return _pimpl->reposToEnable.size(); }

  ServiceInfo::ReposToEnable::const_iterator ServiceInfo::reposToEnableBegin() const
  { return _pimpl->reposToEnable.begin(); }

  ServiceInfo::ReposToEnable::const_iterator ServiceInfo::reposToEnableEnd() const
  { return _pimpl->reposToEnable.end(); }

  bool ServiceInfo::repoToEnableFind( const std::string & alias_r ) const
  { return( _pimpl->reposToEnable.find( alias_r ) != _pimpl->reposToEnable.end() ); }

  void ServiceInfo::addRepoToEnable( const std::string & alias_r )
  {
    _pimpl->reposToEnable.insert( alias_r );
    _pimpl->reposToDisable.erase( alias_r );
  }

  void ServiceInfo::delRepoToEnable( const std::string & alias_r )
  { _pimpl->reposToEnable.erase( alias_r ); }

  void ServiceInfo::clearReposToEnable()
  { _pimpl->reposToEnable.clear(); }


  bool ServiceInfo::reposToDisableEmpty() const
  { return _pimpl->reposToDisable.empty(); }

  ServiceInfo::ReposToDisable::size_type ServiceInfo::reposToDisableSize() const
  { return _pimpl->reposToDisable.size(); }

  ServiceInfo::ReposToDisable::const_iterator ServiceInfo::reposToDisableBegin() const
  { return _pimpl->reposToDisable.begin(); }

  ServiceInfo::ReposToDisable::const_iterator ServiceInfo::reposToDisableEnd() const
  { return _pimpl->reposToDisable.end(); }

  bool ServiceInfo::repoToDisableFind( const std::string & alias_r ) const
  { return( _pimpl->reposToDisable.find( alias_r ) != _pimpl->reposToDisable.end() ); }

  void ServiceInfo::addRepoToDisable( const std::string & alias_r )
  {
    _pimpl->reposToDisable.insert( alias_r );
    _pimpl->reposToEnable.erase( alias_r );
  }

  void ServiceInfo::delRepoToDisable( const std::string & alias_r )
  { _pimpl->reposToDisable.erase( alias_r ); }

  void ServiceInfo::clearReposToDisable()
  { _pimpl->reposToDisable.clear(); }


  std::ostream & ServiceInfo::dumpAsIniOn( std::ostream & str ) const
  {
    RepoInfoBase::dumpAsIniOn(str)
      << "url = " << url() << endl
      << "type = " << type() << endl;

    if ( ! reposToEnableEmpty() )
      str << "repostoenable = " << str::joinEscaped( reposToEnableBegin(), reposToEnableEnd() ) << endl;
    if ( ! reposToDisableEmpty() )
      str << "repostodisable = " << str::joinEscaped( reposToDisableBegin(), reposToDisableEnd() ) << endl;
    return str;
  }

  ostream & ServiceInfo::dumpAsXmlOn( ostream & str, const string & content ) const
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
