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
#include "zypp2/repo/RepoException.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////
 
    RepoException::RepoException()
    : Exception( "Repo exception" )
    {}

    RepoException::RepoException( const std::string & msg_r )
    : Exception( msg_r )
    {}

    RepoException::~RepoException() throw()
    {}
    
    RepoNotCachedException::RepoNotCachedException()
    : RepoException( "Repository not Cached" )
    {}

    RepoNotCachedException::RepoNotCachedException( const std::string & msg_r )
    : RepoException( msg_r )
    {}

    RepoNotCachedException::~RepoNotCachedException() throw()
    {}


    std::ostream & RepoException::dumpOn( std::ostream & str ) const
    {
      return Exception::dumpOn( str );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
