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
 
    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : RepoException::RepoException
    //	METHOD TYPE : Ctor
    //
    RepoException::RepoException()
    : Exception( "Repo exception" )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : RepoException::RepoException
    //	METHOD TYPE : Ctor
    //
    RepoException::RepoException( const std::string & msg_r )
    : Exception( msg_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : RepoException::~RepoException
    //	METHOD TYPE : Dtor
    //
    RepoException::~RepoException() throw()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : RepoException::dumpOn
    //	METHOD TYPE : std::ostream &
    //
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
