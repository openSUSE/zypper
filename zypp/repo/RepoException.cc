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
//#include "zypp/base/Logger.h"
#include "zypp/repo/RepoException.h"
#include "zypp/base/String.h"

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

    RepoException::RepoException( const RepoInfo & info,
        const std::string& msg_r )
    : Exception( msg_r ), _info( info )
    {}

    RepoNotCachedException::RepoNotCachedException( const RepoInfo& info )
    : RepoException( info, "Repository not Cached" )
    {}

    RepoNotCachedException::RepoNotCachedException(  const RepoInfo& info,
        const std::string & msg_r )
    : RepoException( info, msg_r )
    {}

    RepoUnknownTypeException::RepoUnknownTypeException( const RepoInfo &info)
    : RepoException( info,
        str::form("Cannot determine type for repository %s.",info.alias().c_str()))
    {}

    std::ostream & RepoException::dumpOn( std::ostream & str ) const
    {
      return Exception::dumpOn( str );
    }

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
    CLASS::CLASS()                                                           : ServiceException( MSG ) {} \
    CLASS::CLASS( const std::string & msg_r )                                : ServiceException( msg_r ) {} \
    CLASS::CLASS( const ServiceInfo & service_r )                            : ServiceException( service_r, MSG ) {} \
    CLASS::CLASS( const ServiceInfo & service_r, const std::string & msg_r ) : ServiceException( service_r, msg_r ) {}

    DEF_CTORS( ServiceNoAliasException,       "Service has no alias defined." );
    DEF_CTORS( ServiceAlreadyExistsException, "Service already exists." );
    DEF_CTORS( ServiceNoUrlException,         "Service has no or invalid url defined." );

#undef DEF_CTORS

   /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
