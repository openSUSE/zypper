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

    typedef Exception regex_error;
    
    class smatch;
    class regex;
    
    bool regex_match(const std::string& s, str::smatch& matches, const regex& regex);
    bool regex_match(const std::string& s,  const regex& regex);
    
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

      void assign(const std::string& s,int flags = match_extended);
      
    private:
      friend class smatch;
      friend bool regex_match(const std::string& s, str::smatch& matches, const regex& regex);
      friend bool regex_match(const std::string& s,  const regex& regex);
      regex_t m_preg;
      bool m_valid;
    };

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
