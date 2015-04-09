/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/Logger.h
 *
*/
#ifndef ZYPP_BASE_LOGGER_H
#define ZYPP_BASE_LOGGER_H
#include <cstring>
#include <iosfwd>
#include <string>

///////////////////////////////////////////////////////////////////
#ifdef ZYPP_NDEBUG
#define OSDLOG( MSG )
#define OSMLOG( L, MSG )
#else
namespace zypp
{
  namespace debug
  {
    void osdlog( const std::string & msg_r, unsigned level_r );	// LogControl.cc
  }
}
#define OSDLOG( MSG )    ::zypp::debug::osdlog( MSG, 0 )
#define OSMLOG( L, MSG ) ::zypp::debug::osdlog( MSG, L )
#endif // ZYPP_NDEBUG
///////////////////////////////////////////////////////////////////

/** \defgroup ZYPP_BASE_LOGGER_MACROS ZYPP_BASE_LOGGER_MACROS
 *  Convenience macros for logging.
 *
 * The macros finaly call @ref getStream, providing appropriate arguments,
 * to return the log stream.
 *
 * @code
 * _DBG("foo") << ....
 * @endcode
 * Logs a debug message for group @a "foo".
 *
 * @code
 * #undef ZYPP_BASE_LOGGER_LOGGROUP
 * #define ZYPP_BASE_LOGGER_LOGGROUP "foo"
 *
 * DBG << ....
 * @endcode
 * Defines group @a "foo" as default for log messages and logs a
 * debug message.
 */
/*@{*/

#ifndef ZYPP_BASE_LOGGER_LOGGROUP
/** Default log group if undefined. */
#define ZYPP_BASE_LOGGER_LOGGROUP "DEFINE_LOGGROUP"
#endif

#define XXX _XXX( ZYPP_BASE_LOGGER_LOGGROUP )
#define DBG _DBG( ZYPP_BASE_LOGGER_LOGGROUP )
#define MIL _MIL( ZYPP_BASE_LOGGER_LOGGROUP )
#define WAR _WAR( ZYPP_BASE_LOGGER_LOGGROUP )
#define ERR _ERR( ZYPP_BASE_LOGGER_LOGGROUP )
#define SEC _SEC( ZYPP_BASE_LOGGER_LOGGROUP )
#define INT _INT( ZYPP_BASE_LOGGER_LOGGROUP )
#define USR _USR( ZYPP_BASE_LOGGER_LOGGROUP )

#define _XXX(GROUP) ZYPP_BASE_LOGGER_LOG( GROUP, zypp::base::logger::E_XXX )
#define _DBG(GROUP) ZYPP_BASE_LOGGER_LOG( GROUP"++", zypp::base::logger::E_MIL )
#define _MIL(GROUP) ZYPP_BASE_LOGGER_LOG( GROUP, zypp::base::logger::E_MIL )
#define _WAR(GROUP) ZYPP_BASE_LOGGER_LOG( GROUP, zypp::base::logger::E_WAR )
#define _ERR(GROUP) ZYPP_BASE_LOGGER_LOG( GROUP, zypp::base::logger::E_ERR )
#define _SEC(GROUP) ZYPP_BASE_LOGGER_LOG( GROUP, zypp::base::logger::E_SEC )
#define _INT(GROUP) ZYPP_BASE_LOGGER_LOG( GROUP, zypp::base::logger::E_INT )
#define _USR(GROUP) ZYPP_BASE_LOGGER_LOG( GROUP, zypp::base::logger::E_USR )

#define _BASEFILE ( *__FILE__ == '/' ? strrchr( __FILE__, '/' ) + 1 : __FILE__ )

/** Actual call to @ref getStream. */
#define ZYPP_BASE_LOGGER_LOG(GROUP,LEVEL) \
        zypp::base::logger::getStream( GROUP, LEVEL, _BASEFILE, __FUNCTION__, __LINE__ )

/*@}*/

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace logger
    { /////////////////////////////////////////////////////////////////

      /** Definition of log levels.
       *
       * @see getStream
      */
      enum LogLevel {
        E_XXX = 999, /**< Excessive logging. */
        E_DBG = 0,   /**< Debug or verbose. */
        E_MIL,       /**< Milestone. */
        E_WAR,       /**< Warning. */
        E_ERR,       /**< Error. */
        E_SEC,       /**< Secutrity related. */
        E_INT,       /**< Internal error. */
        E_USR        /**< User log. */
      };

      /** Return a log stream to write on.
       *
       * The returned log stream is determined by @a group_r and
       * @a level_r. The remaining arguments @a file_r, @a func_r
       * and @a line_r are expected to denote the location in the
       * source code that issued the message.
       *
       * @note You won't call @ref getStream directly, but use the
       * @ref ZYPP_BASE_LOGGER_MACROS.
      */
      extern std::ostream & getStream( const char * group_r,
                                       LogLevel     level_r,
                                       const char * file_r,
                                       const char * func_r,
                                       const int    line_r );
      extern bool isExcessive();

      /////////////////////////////////////////////////////////////////
    } // namespace logger
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_LOGGER_H
