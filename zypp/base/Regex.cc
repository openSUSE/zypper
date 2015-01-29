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

#include "zypp/base/Regex.h"

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

bool zypp::str::regex_match(const char * s, smatch& matches, const regex& regex)
{
  bool r = s && regex.m_valid && !regexec(&regex.m_preg, s, 12, &matches.pmatch[0], 0);
  if (r)
    matches.match_str = s;
  return r;
}

bool zypp::str::regex_match(const char * s,  const regex& regex)
{
  return s && !regexec(&regex.m_preg, s, 0, NULL, 0);
}

smatch::smatch()
{
  memset(&pmatch, -1, sizeof(pmatch));
}

std::string smatch::operator[](unsigned i) const
{
  if ( i < sizeof(pmatch)/sizeof(*pmatch) && pmatch[i].rm_so != -1 )
    return match_str.substr( pmatch[i].rm_so, pmatch[i].rm_eo-pmatch[i].rm_so );

  return std::string();
}

std::string::size_type smatch::begin( unsigned i ) const
{ return( i < sizeof(pmatch)/sizeof(*pmatch) && pmatch[i].rm_so != -1 ? pmatch[i].rm_so : std::string::npos ); }

std::string::size_type smatch::end( unsigned i ) const
{ return( i < sizeof(pmatch)/sizeof(*pmatch) && pmatch[i].rm_so != -1 ? pmatch[i].rm_eo : std::string::npos ); }

std::string::size_type smatch::size( unsigned i ) const
{ return( i < sizeof(pmatch)/sizeof(*pmatch) && pmatch[i].rm_so != -1 ? pmatch[i].rm_eo-pmatch[i].rm_so : std::string::npos ); }

unsigned smatch::size() const
{
  unsigned matches = unsigned(-1);
  // Get highest (pmatch[i].rm_so != -1). Just looking for the 1st
  // (pmatch[i].rm_so == -1) is wrong as optional mayches "()?"
  // may be embeded.
  for ( unsigned i = 0; i < sizeof(pmatch)/sizeof(*pmatch); ++i )
  {
    if ( pmatch[i].rm_so != -1 )
      matches = i;
  }
  return ++matches;
}
