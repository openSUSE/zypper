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

#include <zypp-core/base/Regex.h>
#include <zypp-core/base/StringV.h>

using namespace zypp;
using namespace zypp::str;

regex::regex()
: m_flags( match_extended )
{}

regex::regex( const std::string & str, int flags )
{ assign( str, flags ); }

regex::~regex()
{
  if ( m_valid )
    regfree( &m_preg );
}

void regex::assign( const std::string & str, int flags )
{
  if ( m_valid )
    regfree( &m_preg );

  m_valid = false;
  m_str = str;
  m_flags = flags;

  static constexpr int normal = 1<<16; // deprecated legacy, use match_extended
  if ( flags & normal ) flags &= ~normal;
  if ( (flags & rxdefault) != rxdefault ) flags |= rxdefault; // always enforced (legacy)

  if ( int err = regcomp( &m_preg, str.c_str(), flags ) ) {
    char errbuff[100];
    regerror( err, &m_preg, errbuff, sizeof(errbuff) );
    ZYPP_THROW( regex_error(std::string(errbuff)) );
  }
  m_valid = true;
}

bool regex::matches( const char* s, smatch & matches, int flags ) const
{
  if ( m_valid ) {
    const auto possibleMatchCount = m_preg.re_nsub + 1;
    matches.pmatch.resize( possibleMatchCount );
    memset( matches.pmatch.data(), -1, sizeof( regmatch_t ) * ( possibleMatchCount ) );

    if ( s && !regexec( &m_preg, s, matches.pmatch.size(), matches.pmatch.data(), flags ) ) {
      matches.match_str = s;
      return true;	// good match
    }
  }
  // Here: no match
  matches.pmatch.clear();
  matches.match_str.clear();
  return false;
}

bool regex::matches( const char* s ) const
{
  return m_valid && s && !regexec( &m_preg, s, 0, NULL, 0 );
}

bool zypp::str::regex_match( const char* s, smatch& matches, const regex& regex )
{ return regex.matches( s, matches ); }

bool zypp::str::regex_match( const char* s,  const regex& regex )
{ return regex.matches( s ); }

smatch::smatch()
{}

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
  strv::splitRx( s, regex, [&result,&replacement,global]( std::string_view w, unsigned, bool last ) {
    result += w;
    if ( !last )
      result += replacement;
    return global;
  });
  return result;
}
