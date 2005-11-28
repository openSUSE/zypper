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
#include <boost/regex.hpp>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  /** String related utilities and \ref ZYPP_STR_REGEX.
   \see \ref ZYPP_STR_REGEX
  */
  namespace str
  { /////////////////////////////////////////////////////////////////

    /** Printf style construction of std::string. */
    std::string form( const char * format, ... )
    __attribute__ ((format (printf, 1, 2)));

    /** Return string describing the \a error_r code.
     * Like ::strerror, but the numerical value is included in
     * the string as well.
    */
    std::string strerror( int errno_r );

    /** \defgroup ZYPP_STR_REGEX Regular expressions
     *
     * Namespace zypp::str regular expressions \b using the
     * boost regex library
     * \url http://www.boost.org/libs/regex/doc/index.html.
     *
     * \li \c regex
     * \li \c regex_match
     * \li \c regex_search
     * \li \c regex_replace
     * \li \c match_results
     * \li \c cmatch
     * \li \c wcmatch
     * \li \c smatch
     * \li \c wsmatch
    */

    //@{
    /** regex */
    using boost::regex;
    using boost::regex_match;
    using boost::regex_search;
    using boost::regex_replace;
    using boost::match_results;
    using boost::cmatch;
    using boost::wcmatch;
    using boost::smatch;
    using boost::wsmatch;
    //@}

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
