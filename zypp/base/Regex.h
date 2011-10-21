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

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  /** String related utilities and \ref ZYPP_STR_REGEX.
   \see \ref ZYPP_STR_REGEX
  */
  namespace str
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /** \defgroup ZYPP_STR_REGEX Regular expressions
     *
     * \brief Namespace zypp::str regular expressions \b using the glibc regex library.
     * \see also \ref sat::AttrMatcher string matcher supporing regex, globing, etc.
     *
     * \code
     *  str::regex rxexpr( "^(A)?([0-9]*) im" );
     *  str::smatch what;
     *
     *  std::string mytext( "Y123 imXXXX" );
     *  if ( str::regex_match( mytext, what, rxexpr ) )
     *  {
     *    MIL << "MATCH '" << what[0] << "'" << endl;
     *    MIL << " subs: " << what.size()-1 << endl;
     *    for_( i, 1U, what.size() )
     *      MIL << "      [" << i << "] " << what[i] << endl;
     *  }
     *  else
     *  {
     *    WAR << "NO MATCH '" << rxexpr << "' in '" <<  mytext << endl;
     *  }
     * \endcode
     */

    typedef Exception regex_error;

    class smatch;
    class regex;

    /**
     * \ingroup ZYPP_STR_REGEX
     */
    bool regex_match(const char * s, str::smatch& matches, const regex& regex);
    inline bool regex_match(const std::string& s, str::smatch& matches, const regex& regex)
    { return regex_match( s.c_str(), matches, regex ); }

    /**
     * \ingroup ZYPP_STR_REGEX
     */
    bool regex_match(const char * s, const regex& regex);
    inline bool regex_match(const std::string& s, const regex& regex)
    { return regex_match( s.c_str(), regex ); }

    /**
     * \ingroup ZYPP_STR_REGEX
     */
    class regex {
    public:

      enum RegFlags {
        optimize = 0,
        match_extra = 0,
        icase = REG_ICASE,
        nosubs = REG_NOSUB,
        match_extended = REG_EXTENDED,
        normal = 1<<16
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

    /**
     * \ingroup ZYPP_STR_REGEX
     */
    class smatch {
    public:
      smatch();

      std::string operator[](unsigned i) const;

      unsigned size() const;

      std::string match_str;
      regmatch_t pmatch[12];
    };



    /////////////////////////////////////////////////////////////////
  } // namespace str
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_STRING_H
