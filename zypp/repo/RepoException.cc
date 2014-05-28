/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/RepoException.cc
 *
*/
#include <iostream>
#include "zypp/repo/RepoException.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    // Repository related exceptions
    //
    ///////////////////////////////////////////////////////////////////

    RepoException::RepoException()
    : Exception( "Repo exception" )
    {}

    RepoException::RepoException( const std::string & msg_r )
    : Exception( msg_r )
    {}

    RepoException::RepoException( const RepoInfo & info )
    : Exception( "Repo exception" ), _info( info )
    {}

    RepoException::RepoException( const RepoInfo & info, const std::string& msg_r )
    : Exception( msg_r ), _info( info )
    {}

    RepoException::~RepoException() throw()
    {}

    std::ostream & RepoException::dumpOn( std::ostream & str ) const
    {
      str << "[" << _info.alias() << "|" << _info.url() << "] ";
      return Exception::dumpOn( str );
    }

    ///////////////////////////////////////////////////////////////////

#define DEF_CTORS( CLASS, MSG ) \
    CLASS::CLASS()                                                        : RepoException( MSG ) {} \
    CLASS::CLASS( const std::string & msg_r )                             : RepoException( msg_r ) {} \
    CLASS::CLASS( const RepoInfo & service_r )                            : RepoException( service_r, MSG ) {} \
    CLASS::CLASS( const RepoInfo & service_r, const std::string & msg_r ) : RepoException( service_r, msg_r ) {}

    DEF_CTORS( RepoNotCachedException,      "Repository is not cached" );
    DEF_CTORS( RepoNoUrlException,          "Repository has no or invalid url defined." );
    DEF_CTORS( RepoNoAliasException,        "Repository has no alias defined." );
    DEF_CTORS( RepoInvalidAliasException,   "Repository has an invalid alias." );
    DEF_CTORS( RepoNotFoundException,       "Repository not found." );
    DEF_CTORS( RepoAlreadyExistsException,  "Repository already exists." );
    DEF_CTORS( RepoUnknownTypeException,    "Repository type can't be determined." );
    DEF_CTORS( RepoMetadataException,       "Repository metadata not usable." );

#undef DEF_CTORS

    ///////////////////////////////////////////////////////////////////
    //
    // Service related exceptions
    //
    ///////////////////////////////////////////////////////////////////

    ServiceException::ServiceException()
    : Exception( "Service exception" )
    {}

    ServiceException::ServiceException( const std::string & msg_r )
    : Exception( msg_r )
    {}

    ServiceException::ServiceException( const ServiceInfo & service_r )
    : Exception( "Service exception" ), _service( service_r )
    {}

    ServiceException::ServiceException( const ServiceInfo & service_r, const std::string & msg_r )
    : Exception( msg_r ), _service( service_r )
    {}

    ServiceException::~ServiceException() throw()
    {}

    std::ostream & ServiceException::dumpOn( std::ostream & str ) const
    {
      str << "[" << _service.alias() << "|" << _service.url() << "] ";
      return Exception::dumpOn( str );
    }

    ///////////////////////////////////////////////////////////////////

#define DEF_CTORS( CLASS, MSG ) \
    CLASS::CLASS()                                                           : DEF_BASECLASS( MSG ) {} \
    CLASS::CLASS( const std::string & msg_r )                                : DEF_BASECLASS( msg_r ) {} \
    CLASS::CLASS( const ServiceInfo & service_r )                            : DEF_BASECLASS( service_r, MSG ) {} \
    CLASS::CLASS( const ServiceInfo & service_r, const std::string & msg_r ) : DEF_BASECLASS( service_r, msg_r ) {}

#define DEF_BASECLASS ServiceException
    DEF_CTORS( ServiceNoAliasException,       "Service has no alias defined." );
    DEF_CTORS( ServiceInvalidAliasException,  "Service has an invalid alias." );
    DEF_CTORS( ServiceAlreadyExistsException, "Service already exists." );
    DEF_CTORS( ServiceNoUrlException,         "Service has no or invalid url defined." );

    // sub classes:
    DEF_CTORS( ServicePluginException,		"PLUGIN service exception." );

    ///////////////////////////////////////////////////////////////////
    // sub class: ServicePluginException
#undef  DEF_BASECLASS
#define DEF_BASECLASS ServicePluginException
    DEF_CTORS( ServicePluginInformalException,	"Service plugin has trouble providing the metadata but this should not be treated as error." );
    DEF_CTORS( ServicePluginImmutableException,	_("Service plugin does not support changing an attribute.") );

#undef DEF_CTORS
   /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
