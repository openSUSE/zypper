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
#include "zypp/base/DefaultIntegral.h"
#include "zypp/parser/xml/XmlEscape.h"

#include "zypp/RepoInfo.h"
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
  struct ServiceInfo::Impl
  {
    typedef ServiceInfo::ReposToEnable  ReposToEnable;
    typedef ServiceInfo::ReposToDisable ReposToDisable;

  public:
    RepoVariablesReplacedUrl _url;
    repo::ServiceType _type;
    ReposToEnable _reposToEnable;
    ReposToDisable _reposToDisable;
    RepoStates _repoStates;
    DefaultIntegral<Date::Duration,0> _ttl;
    Date _lrf;

  public:
    Impl()
    {}

    Impl( const Url & url_r )
    : _url( url_r )
    {}

    ~Impl()
    {}

    void setProbedType( const repo::ServiceType & type_r ) const
    {
      if ( _type == repo::ServiceType::NONE
           && type_r != repo::ServiceType::NONE )
      {
        // lazy init!
        const_cast<Impl*>(this)->_type = type_r;
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

  Url ServiceInfo::url() const			// Variables replaced
  { return _pimpl->_url.transformed(); }

  Url ServiceInfo::rawUrl() const		// Raw
  { return _pimpl->_url.raw(); }

  void ServiceInfo::setUrl( const Url& url )	// Raw
  { _pimpl->_url.raw() = url; }

  repo::ServiceType ServiceInfo::type() const				{ return _pimpl->_type; }
  void ServiceInfo::setType( const repo::ServiceType & type )		{ _pimpl->_type = type; }
  void ServiceInfo::setProbedType( const repo::ServiceType &t ) const	{ _pimpl->setProbedType( t ); }

  Date::Duration ServiceInfo::ttl() const			{ return _pimpl->_ttl; }
  void ServiceInfo::setTtl( Date::Duration ttl_r )		{ _pimpl->_ttl = ttl_r; }
  void ServiceInfo::setProbedTtl( Date::Duration ttl_r ) const	{ const_cast<ServiceInfo*>(this)->setTtl( ttl_r ); }

  Date ServiceInfo::lrf() const					{ return _pimpl->_lrf; }
  void ServiceInfo::setLrf( Date lrf_r )			{ _pimpl->_lrf = lrf_r; }

  bool ServiceInfo::reposToEnableEmpty() const						{ return _pimpl->_reposToEnable.empty(); }
  ServiceInfo::ReposToEnable::size_type ServiceInfo::reposToEnableSize() const		{ return _pimpl->_reposToEnable.size(); }
  ServiceInfo::ReposToEnable::const_iterator ServiceInfo::reposToEnableBegin() const	{ return _pimpl->_reposToEnable.begin(); }
  ServiceInfo::ReposToEnable::const_iterator ServiceInfo::reposToEnableEnd() const	{ return _pimpl->_reposToEnable.end(); }

  bool ServiceInfo::repoToEnableFind( const std::string & alias_r ) const
  { return( _pimpl->_reposToEnable.find( alias_r ) != _pimpl->_reposToEnable.end() ); }

  void ServiceInfo::addRepoToEnable( const std::string & alias_r )
  {
    _pimpl->_reposToEnable.insert( alias_r );
    _pimpl->_reposToDisable.erase( alias_r );
  }

  void ServiceInfo::delRepoToEnable( const std::string & alias_r )
  { _pimpl->_reposToEnable.erase( alias_r ); }

  void ServiceInfo::clearReposToEnable()
  { _pimpl->_reposToEnable.clear(); }


  bool ServiceInfo::reposToDisableEmpty() const						{ return _pimpl->_reposToDisable.empty(); }
  ServiceInfo::ReposToDisable::size_type ServiceInfo::reposToDisableSize() const	{ return _pimpl->_reposToDisable.size(); }
  ServiceInfo::ReposToDisable::const_iterator ServiceInfo::reposToDisableBegin() const	{ return _pimpl->_reposToDisable.begin(); }
  ServiceInfo::ReposToDisable::const_iterator ServiceInfo::reposToDisableEnd() const	{ return _pimpl->_reposToDisable.end(); }

  bool ServiceInfo::repoToDisableFind( const std::string & alias_r ) const
  { return( _pimpl->_reposToDisable.find( alias_r ) != _pimpl->_reposToDisable.end() ); }

  void ServiceInfo::addRepoToDisable( const std::string & alias_r )
  {
    _pimpl->_reposToDisable.insert( alias_r );
    _pimpl->_reposToEnable.erase( alias_r );
  }

  void ServiceInfo::delRepoToDisable( const std::string & alias_r )
  { _pimpl->_reposToDisable.erase( alias_r ); }

  void ServiceInfo::clearReposToDisable()
  { _pimpl->_reposToDisable.clear(); }


  const ServiceInfo::RepoStates & ServiceInfo::repoStates() const	{ return _pimpl->_repoStates; }
  void ServiceInfo::setRepoStates( RepoStates newStates_r )		{ swap( _pimpl->_repoStates, newStates_r ); }


  std::ostream & operator<<( std::ostream & str, const ServiceInfo::RepoState & obj )
  {
    return str
	<< "enabled=" << obj.enabled << " "
	<< "autorefresh=" << obj.autorefresh << " "
	<< "priority=" << obj.priority;
  }

  std::ostream & ServiceInfo::dumpAsIniOn( std::ostream & str ) const
  {
    RepoInfoBase::dumpAsIniOn(str)
      << "url = " << rawUrl() << endl
      << "type = " << type() << endl;

    if ( ttl() )
      str << "ttl_sec = " << ttl() << endl;

    if ( lrf() )
      str << "lrf_dat = " << lrf().asSeconds() << endl;

    if ( ! repoStates().empty() )
    {
      unsigned cnt = 0U;
      for ( const auto & el : repoStates() )
      {
	std::string tag( "repo_" );
	tag += str::numstring( ++cnt );
	const RepoState & state( el.second );

	str << tag << "=" << el.first << endl
	    << tag << "_enabled=" << state.enabled << endl
	    << tag << "_autorefresh=" << state.autorefresh << endl;
	if ( state.priority != RepoInfo::defaultPriority() )
	  str
	    << tag << "_priority=" << state.priority << endl;
      }
    }

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
      << " type=\"" << type().asString() << "\""
      << " ttl_sec=\"" << ttl() << "\"";

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
