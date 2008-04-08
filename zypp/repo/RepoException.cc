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
