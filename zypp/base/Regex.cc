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
  : m_valid(false)
{

}

void regex::assign(const std::string& str,int flags)
{
  m_valid = true;
  int err;
  char errbuff[100];
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

bool zypp::str::regex_match(const std::string& s, smatch& matches, const regex& regex)
{
  bool r = regex.m_valid && !regexec(&regex.m_preg, s.c_str(), 12, &matches.pmatch[0], 0);
  if (r)
    matches.match_str = s;
  return r;
}

bool zypp::str::regex_match(const std::string& s,  const regex& regex)
{
  return !regexec(&regex.m_preg, s.c_str(), 0, NULL, 0);
}

smatch::smatch()
{
  memset(&pmatch, -1, sizeof(pmatch));
}

std::string smatch::operator[](unsigned i) const 
{
  if (i < sizeof(pmatch)/sizeof(*pmatch) && pmatch[i].rm_so != -1)
    return match_str.substr(pmatch[i].rm_so, pmatch[i].rm_eo-pmatch[i].rm_so);
  return std::string();
}


unsigned smatch::size() const
{
  unsigned matches = 0;
  while (matches <  ((sizeof(pmatch)/sizeof(*pmatch))-1) && pmatch[matches+1].rm_so != -1) {
    //    std::cout << "match[" << matches << "]: *" << (*this)[matches
    //        +1] << "*" << std::endl;
    matches++;
  }

  return matches;
}
