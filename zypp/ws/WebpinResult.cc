/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/WebpinResult.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/ws/WebpinResult.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace ws
{
    
  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : WebpinResult::Impl
  //
  /** WebpinResult implementation. */
  struct WebpinResult::Impl
  {
    Impl()
        : priority(0)
    {}

    ~Impl()
    {
      //MIL << std::endl;
    }
  public:
      std::string name;
      Edition edition;
      CheckSum checksum;
      Url repourl;
      string summary;
      string distro;
      int priority;
      
  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
 
  /** \relates WebpinResult::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const WebpinResult::Impl & obj )
  {
    return str << "WebpinResult::Impl";
  }

  WebpinResult::WebpinResult()
  : _pimpl( new Impl() )
  {}

  WebpinResult::~WebpinResult()
  {
    //MIL << std::endl;
  }

  WebpinResult & WebpinResult::setName( const std::string &name )
  {
    _pimpl->name = name;
    return *this;
  }

  std::string WebpinResult::name() const
  {
      return _pimpl->name;
  }

 
  zypp::Url WebpinResult::repositoryUrl() const
  {
      return _pimpl->repourl;    
  }

  WebpinResult & WebpinResult::setRepositoryUrl( const zypp::Url &url )
  {
      _pimpl->repourl = url;
      return *this;
  }
    
  WebpinResult & WebpinResult::setDistribution( const std::string &distro )
  {
    _pimpl->distro = distro;
    return *this;
  }

  std::string WebpinResult::distribution() const
  {
      return _pimpl->distro;
  }

  WebpinResult & WebpinResult::setSummary( const std::string &summary )
  {
    _pimpl->summary = summary;
    return *this;
  }

  std::string WebpinResult::summary() const
  {
      return _pimpl->summary;
  }

  WebpinResult & WebpinResult::setPriority( int priority )
  {
    _pimpl->priority = priority;
    return *this;
  }

  int WebpinResult::priority() const
  {
      return _pimpl->priority;
  }


  WebpinResult & WebpinResult::setEdition( const Edition &edition )
  {
    _pimpl->edition = edition;
    return *this;
  }

  Edition WebpinResult::edition() const
  {
      return _pimpl->edition;
  }

  WebpinResult & WebpinResult::setChecksum( const CheckSum &checksum )
  {
    _pimpl->checksum = checksum;
    return *this;
  }

  CheckSum WebpinResult::checksum() const
  {
      return _pimpl->checksum;
  }


  std::ostream & WebpinResult::dumpOn( std::ostream & str ) const
  {
    str << "- name        : " << name() << std::endl;
    return str;
  }

  std::ostream & operator<<( std::ostream & str, const WebpinResult & obj )
  {
    return obj.dumpOn(str);
  }

} // namespace ws
    
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
