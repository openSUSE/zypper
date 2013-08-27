/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/Regex.h
 *
*/
#ifndef ZYPP_BASE_REGEX_H
#define ZYPP_BASE_REGEX_H

#include <iosfwd>
#include <string>
#include <regex.h>

#include "zypp/base/Exception.h"

//////////////////////////////////////////////////////////////////
namespace zypp
{
  //////////////////////////////////////////////////////////////////
  /// \namespace str
  /// \brief String related utilities and \ref ZYPP_STR_REGEX.
  namespace str
  {
    //////////////////////////////////////////////////////////////////
    /// \defgroup ZYPP_STR_REGEX Regular expression matching
    /// \brief Regular expressions using the glibc regex library.
    ///
    /// \see also \ref StrMatcher string matcher also supporting globing, etc.
    ///
    /// \code
    ///  str::regex rxexpr( "^(A)?([0-9]*) im" );
    ///  str::smatch what;
    ///
    ///  std::string mytext( "Y123 imXXXX" );
    ///  if ( str::regex_match( mytext, what, rxexpr ) )
    ///  {
    ///    MIL << "MATCH '" << what[0] << "'" << endl;
    ///    MIL << " subs: " << what.size()-1 << endl;
    ///    for_( i, 1U, what.size() )
    ///      MIL << "      [" << i << "] " << what[i] << endl;
    ///  }
    ///  else
    ///  {
    ///    WAR << "NO MATCH '" << rxexpr << "' in '" <<  mytext << endl;
    ///  }
    /// \endcode
    //////////////////////////////////////////////////////////////////

    typedef Exception regex_error;

    class smatch;
    class regex;

    //////////////////////////////////////////////////////////////////
    /// \brief Regular expression matching
    ///
    /// \ingroup ZYPP_STR_REGEX
    /// \relates regex
    /// Return whether a \ref regex matches a specific string. An optionally
    /// passed \ref smatch object will contain the match reults.
    //////////////////////////////////////////////////////////////////
    bool regex_match( const char * s, smatch & matches, const regex & regex );

    /** \copydoc regex_match \relates regex \ingroup ZYPP_STR_REGEX */
    inline bool regex_match(const std::string & s, smatch & matches, const regex & regex)
    { return regex_match( s.c_str(), matches, regex ); }

    /** \copydoc regex_match \relates regex \ingroup ZYPP_STR_REGEX */
    bool regex_match( const char * s, const regex & regex );

    /** \copydoc regex_match \relates regex \ingroup ZYPP_STR_REGEX */
    inline bool regex_match( const std::string & s, const regex & regex )
    { return regex_match( s.c_str(), regex ); }

    //////////////////////////////////////////////////////////////////
    /// \class regex
    /// \brief Regular expression
    ///
    /// \ingroup ZYPP_STR_REGEX
    //////////////////////////////////////////////////////////////////
    class regex
    {
    public:

      enum RegFlags {
        icase		= REG_ICASE,	///< Do not differentiate case
        nosubs		= REG_NOSUB,	///< Support for substring addressing of matches is not required
        match_extended	= REG_EXTENDED, ///< Use POSIX Extended Regular Expression syntax when interpreting regex.
      };

      regex();
      regex(const std::string& s,int flags = match_extended);
      ~regex() throw();

      regex(const regex & rhs)
      { assign(rhs.m_str, rhs.m_flags); }

      regex & operator=(const regex & rhs)
      { assign(rhs.m_str, rhs.m_flags); return *this; }

      /**
       * string representation of the regular expression
       */
      std::string asString() const
      { return m_str; }

    public:
      /** Expert backdoor. Returns pointer to the compiled regex for direct use in regexec() */
      regex_t * get()
      { return & m_preg; }

    private:
      void assign(const std::string& s,int flags = match_extended);

    private:
      friend class smatch;
      friend bool regex_match(const char * s, str::smatch& matches, const regex& regex);
      friend bool regex_match(const char * s,  const regex& regex);
      std::string m_str;
      int m_flags;
      regex_t m_preg;
      bool m_valid;
    };

    /** \relates regex Stream output */
    inline std::ostream & operator<<( std::ostream & str, const regex & obj )
    { return str << obj.asString(); }

    //////////////////////////////////////////////////////////////////
    /// \class smatch
    /// \brief Regular expression match result
    ///
    /// \ingroup ZYPP_STR_REGEX
    ///
    /// Index \c n=0 returns the string object representing the character
    /// sequence that matched the whole regular expression.
    /// If \c n is out of range, or if \c n is an unmatched sub-expression,
    /// then an empty string is returned.
    //////////////////////////////////////////////////////////////////
    class smatch
    {
    public:
      smatch();

      std::string operator[](unsigned i) const;

      unsigned size() const;

      std::string match_str;
      regmatch_t pmatch[12];
    };

  } // namespace str
  //////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_STRING_H
