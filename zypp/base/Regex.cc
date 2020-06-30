/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/Regex.cc
 *
*/
#include <cstdio>
#include <cstdarg>

#include <iostream>

#include <zypp/base/Regex.h>

using namespace zypp;
using namespace zypp::str;

regex::regex()
  : m_flags(match_extended)
  , m_valid(false)
{

}

void regex::assign(const std::string& str,int flags)
{
  m_valid = true;
  m_str = str;
  m_flags = flags;
  int err;
  char errbuff[100];
  static const int normal = 1<<16; // deprecated legacy, use match_extended
  if (!(flags & normal)) {
    flags |= match_extended;
    flags &= ~(normal);
  }

  if ((err = regcomp(&m_preg, str.c_str(), flags))) {
    m_valid = false;
    regerror(err, &m_preg, errbuff, sizeof(errbuff));
    ZYPP_THROW(regex_error(std::string(errbuff)));
  }
}

regex::regex(const std::string& str, int flags)
{
  assign(str, flags);
}

regex::~regex() throw()
{
  if (m_valid)
    regfree(&m_preg);
}

bool regex::matches( const char *s, smatch &matches, int flags ) const
{
  const auto possibleMatchCount = m_preg.re_nsub + 1;
  matches.pmatch.resize( possibleMatchCount );
  memset( matches.pmatch.data(), -1, sizeof( regmatch_t ) * ( possibleMatchCount ) );

  bool r = s && m_valid && !regexec( &m_preg, s, matches.pmatch.size(), matches.pmatch.data(), flags );
  if (r)
    matches.match_str = s;
  return r;
}

bool regex::matches( const char *s ) const
{
  return s && !regexec(&m_preg, s, 0, NULL, 0);
}

bool zypp::str::regex_match(const char * s, smatch& matches, const regex& regex)
{
  return regex.matches( s, matches );
}

bool zypp::str::regex_match(const char * s,  const regex& regex)
{
  return regex.matches( s );
}

smatch::smatch()
{ }

std::string smatch::operator[](unsigned i) const
{
  if ( i < pmatch.size() && pmatch[i].rm_so != -1 )
    return match_str.substr( pmatch[i].rm_so, pmatch[i].rm_eo-pmatch[i].rm_so );

  return std::string();
}

std::string::size_type smatch::begin( unsigned i ) const
{ return( i < pmatch.size() && pmatch[i].rm_so != -1 ? pmatch[i].rm_so : std::string::npos ); }

std::string::size_type smatch::end( unsigned i ) const
{ return( i < pmatch.size() && pmatch[i].rm_so != -1 ? pmatch[i].rm_eo : std::string::npos ); }

std::string::size_type smatch::size( unsigned i ) const
{ return( i < pmatch.size() && pmatch[i].rm_so != -1 ? pmatch[i].rm_eo-pmatch[i].rm_so : std::string::npos ); }

unsigned smatch::size() const
{
  unsigned matches = unsigned(-1);
  // Get highest (pmatch[i].rm_so != -1). Just looking for the 1st
  // (pmatch[i].rm_so == -1) is wrong as optional mayches "()?"
  // may be embeded.
  for ( unsigned i = 0; i < pmatch.size(); ++i )
  {
    if ( pmatch[i].rm_so != -1 )
      matches = i;
  }
  return ++matches;
}

std::string zypp::str::regex_substitute( const std::string &s, const regex &regex, const std::string &replacement, bool global )
{
  std::string result;
  std::string::size_type off = 0;
  int flags = regex::none;

  while ( true ) {

    smatch match;
    if ( !regex.matches( s.data()+off, match, flags ) ) {
      break;
    }

    if ( match.size() ) {
      result += s.substr( off, match.begin(0) );
      result += replacement;
      off = match.end(0) + off;
    }

    if ( !global )
      break;

    // once we passed the beginning of the string we should not match ^ anymore, except the last character was
    // actually a newline
    if ( off > 0 && off < s.size() && s[ off - 1 ] == '\n' )
      flags = regex::none;
    else
      flags = regex::not_bol;
  }

  result += s.substr( off );
  return result;
}
