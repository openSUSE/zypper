/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/String.h
 *
*/
#ifndef ZYPP_BASE_STRING_H
#define ZYPP_BASE_STRING_H

#include <iosfwd>
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace str
  { /////////////////////////////////////////////////////////////////

    /** Printf style construction of std::string. */
    std::string form( const char * format, ... )
    __attribute__ ((format (printf, 1, 2)));

    /** Return lowercase version of \a s
     * \todo improve
    */
    std::string toLower( const std::string & s );

    /** Return uppercase version of \a s
     * \todo improve
    */
    std::string toUpper( const std::string & s );

    /////////////////////////////////////////////////////////////////
  } // namespace str
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_STRING_H
