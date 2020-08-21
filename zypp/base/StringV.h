/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/StringV.h
 * c++17: std::string_view tools
 */
#ifndef ZYPP_BASE_STRINGV_H
#define ZYPP_BASE_STRINGV_H
#include <string_view>
#ifdef __cpp_lib_string_view

#include <zypp/base/String.h>
#include <zypp/base/Flags.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{
  namespace strv
  {
    /** Define how to trim. */
    enum class Trim {
      notrim = 0,
      right  = 1<<0,
      left   = 1<<1,
      trim   = (left|right),
    };

    ZYPP_DECLARE_FLAGS_AND_OPERATORS( TrimFlag, Trim );


    /** Split \a line_r into words separated by \a sep_r and invoke \a fnc_r with each word.
     *
     * If \a sep_r is not empty, each separator found in \a line_r will be enclosed by 2
     * words being reported. Words may be empty if the separator is located at the beginning
     * or the end of \a line_r of it there are consecutive occurrences. If the separator does
     * not occur on the line the whole string is reported.
     *
     * If \a sep_r is unspecified or empty, it splits on whitespace /[BLANK,TAB]+/. In this
     * case only the (not empty) words found on the line are reported.
     *
     * The optional \a trim_r argument tells whether whitespace around the words found
     * should be trimmed before reporting them. The default is not to trim.
     *
     * \returns the number of words reported.
     *
     * \code
     *   str = ""
     *
     *   split( str, fnc );
     *   // []
     *
     *   split( str, " ", fnc );
     *   // ['']
     *
     *
     *   str = " "
     *
     *   split( str, fnc );
     *   // []
     *
     *   split( str, " ", fnc );
     *   // ['', '']
     *
     *
     *   str = " 1 2 3 4 5 ";
     *
     *   split( str, fnc );
     *   // ['1', '2', '3', '4', '5']
     *
     *   split( str, " ", fnc );
     *   // ['', '1', '2', '3', '4', '5', '']
     *
     *   split( str, " 2", fnc );
     *   // [' 1', ' 3 4 5 ']
     *
     *   split( str, " 2", Trim::all, fnc );
     *   // ['1', '3 4 5']
     * \endcode
     */
    unsigned split( std::string_view line_r, std::string_view sep_r, Trim trim_r,
		    std::function<void(std::string_view)> fnc_r );

    /** \overload  Split at \a sep_r and Trim::notrim */
    inline unsigned split( std::string_view line_r, std::string_view sep_r,
		    std::function<void(std::string_view)> fnc_r )
    { return split( line_r, sep_r, Trim::notrim, fnc_r ); }

    /** \overload  Split at whitespace */
    inline unsigned split( std::string_view line_r,
		    std::function<void(std::string_view)> fnc_r )
    { return split( line_r, std::string_view(), Trim::notrim, fnc_r ); }

  } // namespace strv
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // __cpp_lib_string_view
#endif // ZYPP_BASE_STRINGV_H
