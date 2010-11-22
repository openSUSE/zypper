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
   * of a certain kind.
   * \code
   *     ProgressData ticks( makeProgressData( input_r ) );
   *     ticks.sendTo( fnc_r );
   *     ticks.toMin(); // start sending min (0)
   *
   *     iostr::EachLine line( input_r );
   *     for( ; line; line.next() )
   *     {
   *       // process the line
   *
   *       if ( ! ticks.set( input_r.stream().tellg() ) )
   *         ZYPP_THROW( AbortRequestException( "" ) );
   *     }
   * \endcode
   * \code
   * // either this way
   * catch ( const AbortRequestException & excpt_r )
   * {
   *   ...
   * }
   *
   * // or that
   * catch ( const UserRequestException & excpt_r )
   * {
   *   switch ( excpt_r.kind() )
   *   {
   *     case UserRequestException::ABORT:
   *       ...
   *       break;
   *   }
   * }
   * \endcode
  */
  class UserRequestException : public Exception
  {
    public:
      enum Kind { UNSPECIFIED, IGNORE, SKIP, RETRY, ABORT };
    public:
      explicit
      UserRequestException( const std::string & msg_r = std::string() );
      UserRequestException( const std::string & msg_r, const Exception & history_r );
      explicit
      UserRequestException( Kind kind_r, const std::string & msg_r = std::string() );
      UserRequestException( Kind kind_r, const std::string & msg_r, const Exception & history_r );
    public:
      Kind kind() const
      { return _kind; }
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
      Kind _kind;
  };
  ///////////////////////////////////////////////////////////////////

  /** Convenience macro to declare more specific PluginScriptExceptions. */
#define declException( EXCP, KIND )					\
  struct EXCP : public UserRequestException {				\
    explicit								\
    EXCP( const std::string & msg_r = std::string() )			\
      : UserRequestException( KIND, msg_r )				\
    {}									\
    EXCP( const std::string & msg_r, const Exception & history_r )	\
      : UserRequestException( KIND, msg_r, history_r )			\
    {}									\
  }

  declException( IgnoreRequestException, IGNORE );
  declException( SkipRequestException, SKIP );
  declException( RetryRequestException, RETRY );
  declException( AbortRequestException, ABORT );

#undef declException

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_USERREQUESTEXCEPTION_H
