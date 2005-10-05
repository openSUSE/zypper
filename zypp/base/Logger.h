/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 \file	zypp/base/Logger.h

 \brief	.

*/
#ifndef ZYPP_BASE_LOGGER_H
#define ZYPP_BASE_LOGGER_H

#include <iosfwd>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace logger
    { /////////////////////////////////////////////////////////////////

      enum LogLevel {
        E_USR = 0,
        E_DBG, E_MIL, E_WAR, E_ERR, E_SEC, E_INT
      };

      extern std::ostream & getStream( LogLevel level_r );

      /////////////////////////////////////////////////////////////////
    } // namespace logger
    ///////////////////////////////////////////////////////////////////

#define USR zypp::base::logger::getStream( zypp::base::logger::E_USR )
#define DBG zypp::base::logger::getStream( zypp::base::logger::E_DBG )
#define MIL zypp::base::logger::getStream( zypp::base::logger::E_MIL )
#define WAR zypp::base::logger::getStream( zypp::base::logger::E_WAR )
#define ERR zypp::base::logger::getStream( zypp::base::logger::E_ERR )
#define SEC zypp::base::logger::getStream( zypp::base::logger::E_SEC )
#define INT zypp::base::logger::getStream( zypp::base::logger::E_INT )

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_LOGGER_H
