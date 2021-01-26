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

#include <zypp-core/base/String.h>
#include <zypp-core/base/Regex.h>
#include <zypp-core/base/Flags.h>

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
      left   = 1<<0,
      right  = 1<<1,
      trim   = (left|right),
    };

    ZYPP_DECLARE_FLAGS_AND_OPERATORS( TrimFlag, Trim );

    /** The default blank. */
    inline constexpr std::string_view blank = " \t";

    /** Trim \a chars_r at the beginning of \a str_r. */
    inline std::string_view ltrim( std::string_view str_r, std::string_view chars_r = blank )
    {
      if ( str_r.empty() )
	return str_r;
      auto pos = str_r.find_first_not_of( chars_r );
      if ( pos == str_r.npos )
	str_r.remove_prefix( str_r.size() );
      else if ( pos )
	str_r.remove_prefix( pos );
      return str_r;
    }

    /** Trim \a chars_r at the end of \a str_r. */
    inline std::string_view rtrim( std::string_view str_r, std::string_view chars_r = blank )
    {
      if ( str_r.empty() )
	return str_r;
      auto pos = str_r.find_last_not_of( chars_r );
      if ( pos == str_r.npos )
	str_r.remove_suffix( str_r.size() );
      else if ( (pos = str_r.size()-1-pos) )
	str_r.remove_suffix( pos );
      return str_r;
    }

    /** Trim \a chars_r at both sides of \a str_r. */
    inline std::string_view trim( std::string_view str_r, std::string_view chars_r = blank )
    {
      if ( str_r.empty() )
	return str_r;
      str_r = ltrim( std::move(str_r), chars_r );
      str_r = rtrim( std::move(str_r), chars_r );
      return str_r;
    }

    /** Trim \a chars_r at \a trim_r sides of \a str_r. */
    inline std::string_view trim( std::string_view str_r, std::string_view chars_r, TrimFlag trim_r )
    {
      if ( str_r.empty() || trim_r == Trim::notrim )
	return str_r;
      if ( trim_r.testFlag( Trim::left ) )
	str_r = ltrim( std::move(str_r), chars_r );
      if ( trim_r.testFlag( Trim::right ) )
	str_r = rtrim( std::move(str_r), chars_r );
      return str_r;
    }
    /** \overload Trimming blanks at \a trim_r sides of \a str_r. */
    inline std::string_view trim( std::string_view str_r, TrimFlag trim_r )
    { return trim( std::move(str_r), blank, std::move(trim_r) ); }

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

      /** \ref split working horse */
      unsigned _split( std::string_view line_r, std::string_view sep_r, Trim trim_r, WordConsumer && fnc_r );

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
     * (or no callback at all)
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
     * - If \a sep_r is not empty, each separator found in \a line_r will be enclosed by 2
     * words being reported. Words may be empty if the separator is located at the beginning
     * or the end of \a line_r or it there are consecutive occurrences.
     *
     * - If \a sep_r is unspecified or empty, it splits on whitespace /[BLANK,TAB]+/. In this
     * case only the (not empty) words found on the line are reported (trimmed).
     *
     * The optional \a trim_r argument tells whether whitespace around the words found
     * should be trimmed before reporting them. The default is not to trim.
     *
     * Accepted callbacks: [bool|void]( [std::string_view[, unsigned[, bool]]] )
     * (or no callback at all)
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
    template <typename Callable = detail::WordConsumer>
    unsigned split( std::string_view line_r, std::string_view sep_r, Trim trim_r, Callable && fnc_r = Callable() )
    { return detail::_split( line_r, sep_r, trim_r, detail::wordConsumer( std::forward<Callable>(fnc_r) ) ); }

    /** \overload  Split at \a sep_r and Trim::notrim */
    template <typename Callable = detail::WordConsumer>
    inline unsigned split( std::string_view line_r, std::string_view sep_r, Callable && fnc_r = Callable() )
    { return detail::_split( line_r, sep_r, Trim::notrim, detail::wordConsumer( std::forward<Callable>(fnc_r) ) ); }

    /** \overload  Split at whitespace */
    template <typename Callable = detail::WordConsumer>
    inline unsigned split( std::string_view line_r, Callable && fnc_r = Callable() )
    { return detail::_split( line_r, std::string_view(), Trim::notrim, detail::wordConsumer( std::forward<Callable>(fnc_r) ) ); }

  } // namespace strv
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // __cpp_lib_string_view
#endif // ZYPP_BASE_STRINGV_H
