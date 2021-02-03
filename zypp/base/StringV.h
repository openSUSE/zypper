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
#include <zypp/base/Regex.h>
#include <zypp/base/Flags.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{
  namespace strv
  {
    using regex = str::regex;
    using smatch = str::smatch;

    /** Define how to trim. */
    enum class Trim {
      notrim = 0,
      right  = 1<<0,
      left   = 1<<1,
      trim   = (left|right),
    };

    ZYPP_DECLARE_FLAGS_AND_OPERATORS( TrimFlag, Trim );

    ///////////////////////////////////////////////////////////////////
    namespace detail
    {
      /** Split* functions working horse callback. */
      using WordConsumer = std::function<bool(std::string_view,unsigned,bool)>;

      /** \name Split* callback signatures accepted for convenience.
       *
       * Details may vary for specific split* functions, but basically each separator found
       * in line will be enclosed by 2 words being reported. The working horse callback
       * offers the \a word, the \a index of word (starting with 0) and a boolean indicating whether
       * this is going to be the \a last report.
       *
       * A callback may optionally return a value convertible to \c bool. Returning \c false
       * usually causes split* to stop looking further separators and to finish the reporting.
       *
       * Accepted callbacks: [bool|void] callback( [std::string_view[, unsigned[, bool]]] )
       * (or no callback at all )
       */
      //@{
      /** bool(std::string_view,unsigned,bool) */
      template <typename Callable, std::enable_if_t<
      std::is_invocable_r_v<bool,Callable,std::string_view,unsigned,bool>
      , bool> = true>
      WordConsumer wordConsumer( Callable && fnc_r )
      { return std::forward<Callable>(fnc_r); }
      /** void(std::string_view,unsigned,bool) */
      template <typename Callable, std::enable_if_t<
      ! std::is_invocable_r_v<bool,Callable,std::string_view,unsigned,bool>
      && std::is_invocable_r_v<void,Callable,std::string_view,unsigned,bool>
      , bool> = true>
      WordConsumer wordConsumer( Callable && fnc_r )
      { return [&fnc_r](std::string_view a1,unsigned a2,bool a3)->bool { fnc_r(a1,a2,a3); return true; }; }

      /** bool(std::string_view,unsigned) */
      template <typename Callable, std::enable_if_t<
      std::is_invocable_r_v<bool,Callable,std::string_view,unsigned>
      , bool> = true>
      WordConsumer wordConsumer( Callable && fnc_r )
      { return [&fnc_r](std::string_view a1,unsigned a2,bool)->bool { return fnc_r(a1,a2); }; }
      /** void(std::string_view,unsigned) */
      template <typename Callable, std::enable_if_t<
      ! std::is_invocable_r_v<bool,Callable,std::string_view,unsigned>
      && std::is_invocable_r_v<void,Callable,std::string_view,unsigned>
      , bool> = true>
      WordConsumer wordConsumer( Callable && fnc_r )
      { return [&fnc_r](std::string_view a1,unsigned a2,bool)->bool { fnc_r(a1,a2); return true; }; }

      /** bool(std::string_view) */
      template <typename Callable, std::enable_if_t<
      std::is_invocable_r_v<bool,Callable,std::string_view>
      , bool> = true>
      WordConsumer wordConsumer( Callable && fnc_r )
      { return [&fnc_r](std::string_view a1,unsigned,bool)->bool { return fnc_r(a1); }; }
      /** void(std::string_view) */
      template <typename Callable, std::enable_if_t<
      ! std::is_invocable_r_v<bool,Callable,std::string_view>
      && std::is_invocable_r_v<void,Callable,std::string_view>
      , bool> = true>
      WordConsumer wordConsumer( Callable && fnc_r )
      { return [&fnc_r](std::string_view a1,unsigned,bool)->bool { fnc_r(a1); return true; }; }

      /** bool() */
      template <typename Callable, std::enable_if_t<
      std::is_invocable_r_v<bool,Callable>
      , bool> = true>
      WordConsumer wordConsumer( Callable && fnc_r )
      { return [&fnc_r](std::string_view,unsigned,bool)->bool { return fnc_r(); }; }
      /** void() */
      template <typename Callable, std::enable_if_t<
      ! std::is_invocable_r_v<bool,Callable>
      && std::is_invocable_r_v<void,Callable>
      , bool> = true>
      WordConsumer wordConsumer( Callable && fnc_r )
      { return [&fnc_r](std::string_view,unsigned,bool)->bool { fnc_r(); return true; }; }
      //@}

      /** \ref splitRx working horse */
      unsigned _splitRx( const std::string & line_r, const regex & rx_r, WordConsumer && fnc_r );
    }  // namespace detail
    ///////////////////////////////////////////////////////////////////

    /** Split \a line_r into words separated by the regular expression \a rx_r and invoke \a fnc_r with each word.
     *
     * Each separator match found in \a line_r will be enclosed by 2 words being reported.
     * Words may be empty if the separator match is located at the beginning or at the end
     * of \a line_r, or it there are consecutive separator match occurrences.
     *
     * Accepted callbacks: [bool|void]( [std::string_view[, unsigned[, bool]]] )
     * (or no callback at all )
     *
     * A callback may take the \a word, the \a index of word (starting with 0) and a boolean
     * indicating whether this is going to be the \a last report.
     *
     * A callback may optionally return a value convertible to \c bool. Returning \c false
     * causes split to stop looking further separators. The final report will contain the
     * remaining part of the \a line_r then.
     *
     * If the separator does not occur on the line the whole string is reported.
     *
     * \returns the number of words reported.
     */
    template <typename Callable = detail::WordConsumer>
    unsigned splitRx( const std::string & line_r, const regex & rx_r, Callable && fnc_r = Callable() )
    { return detail::_splitRx( line_r, rx_r, detail::wordConsumer( std::forward<Callable>(fnc_r) ) ); }


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
