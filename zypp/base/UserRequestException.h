/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/UserRequestException.h
 *
*/
#ifndef ZYPP_BASE_USERREQUESTEXCEPTION_H
#define ZYPP_BASE_USERREQUESTEXCEPTION_H

#include <iosfwd>

#include "zypp/base/Exception.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : UserRequestException
  //
  /** Base for exceptions caused by explicit user request.
   *
   * Use the derived convenience classes to throw exceptions
   * of a certain kind..
  */
  class UserRequestException : public Exception
  {
    public:
      enum Kind { UNSPECIFIED, IGNORE, SKIP, RETRY, ABORT };
    public:
      explicit
      UserRequestException( const std::string & msg_r = std::string() );
      explicit
      UserRequestException( Kind kind_r, const std::string & msg_r = std::string() );
    public:
      Kind kind() const
      { return _kind; }
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
      Kind _kind;
  };
  ///////////////////////////////////////////////////////////////////

  struct IgnoreRequestException : public UserRequestException
  {
    explicit
    IgnoreRequestException( const std::string & msg_r = std::string() )
      : UserRequestException( IGNORE, msg_r )
    {}
  };

  struct SkipRequestException : public UserRequestException
  {
    explicit
    SkipRequestException( const std::string & msg_r = std::string() )
      : UserRequestException( SKIP, msg_r )
    {}
  };

  struct RetryRequestException : public UserRequestException
  {
    explicit
    RetryRequestException( const std::string & msg_r = std::string() )
      : UserRequestException( RETRY, msg_r )
    {}
  };

  struct AbortRequestException : public UserRequestException
  {
    explicit
    AbortRequestException( const std::string & msg_r = std::string() )
      : UserRequestException( ABORT, msg_r )
    {}
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_USERREQUESTEXCEPTION_H
