/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_REPO_VARIABLES_H_
#define ZYPP_REPO_VARIABLES_H_

#include <iosfwd>
#include <string>

#include "zypp/Url.h"
#include "zypp/Arch.h"

namespace zypp
{
  namespace repo
  {

  /**
   * \short Repository variables
   *
   * ...
   */
  struct RepoVariablesStringReplacer : public std::unary_function<std::string, std::string>
  {
    RepoVariablesStringReplacer();

    std::string operator()( const std::string &value ) const;

    ~RepoVariablesStringReplacer();

    void resetVarCache( void );

    private:
      mutable Arch sysarch;
      mutable Arch basearch;
      mutable std::string releasever;
  };

  /**
   * \short Repository variables
   *
   * ...
   */
  struct RepoVariablesUrlReplacer : public std::unary_function<Url, Url>
  {
    RepoVariablesUrlReplacer();

    Url operator()( const Url &url ) const;

    ~RepoVariablesUrlReplacer();

    void resetVarCache( void );

    private:
      RepoVariablesStringReplacer replacer;
  };

  } // ns repo
} // ns zypp

#endif

// vim: set ts=2 sts=2 sw=2 et ai:
