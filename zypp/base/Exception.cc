/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/Exception.cc
 *
*/
#include <iostream>
#include <sstream>

#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/String.h"
#include "zypp/base/Exception.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace exception_detail
  { /////////////////////////////////////////////////////////////////

    std::string CodeLocation::asString() const
    {
      return str::form( "%s(%s):%u",
                        _file.c_str(),
                        _func.c_str(),
                        _line );
    }

    std::ostream & operator<<( std::ostream & str, const CodeLocation & obj )
    { return str << obj.asString(); }

    /////////////////////////////////////////////////////////////////
  } // namespace exception_detail
  ///////////////////////////////////////////////////////////////////

  Exception::Exception()
  {}

  Exception::Exception( const std::string & msg_r )
  : _msg( msg_r )
  {}

  Exception::~Exception() throw()
  {}

  std::string Exception::asString() const
  {
    std::ostringstream str;
    dumpOn( str );
    return str.str();
  }

  std::string Exception::asUserString() const
  {
    std::ostringstream str;
    dumpOn( str );
    return _(str.str().c_str());
  }


  std::ostream & Exception::dumpOn( std::ostream & str ) const
  { return str << _msg; }

  std::ostream & Exception::dumpError( std::ostream & str ) const
  { return dumpOn( str << _where << ": " ); }

  std::ostream & operator<<( std::ostream & str, const Exception & obj )
  { return obj.dumpError( str ); }


  std::string Exception::strErrno( int errno_r )
  { return str::strerror( errno_r ); }

  std::string Exception::strErrno( int errno_r, const std::string & msg_r )
  {
    std::string ret( msg_r );
    ret += ": ";
    return ret += strErrno( errno_r );
  }

  void Exception::log( const Exception & excpt_r, const CodeLocation & where_r,
                       const char *const prefix_r )
  {
    INT << where_r << " " << prefix_r << " " << excpt_r << endl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
