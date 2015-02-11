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

#include <string>
#include "zypp/base/ValueTransform.h"
#include "zypp/Url.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace repo
  {
    /**
     * \short Functor replacing repository variables
     *
     * Replaces '$arch', '$basearch' and $releasever in a string
     * with the global ZYpp values.
     *
     * Additionally $releasever_major and $releasever_minor can be used
     * to refer to $releasever major number (everything up to the 1st \c '.' )
     * and minor number (everything after the 1st \c '.' ).
     *
     * \note The $releasever value is overwritten by the environment
     * variable \c ZYPP_REPO_RELEASEVER. This might  be handy for
     * distribution upogrades like this:
     * \code
     *   $ export ZYPP_REPO_RELEASEVER=13.2
     *   $ zypper lr -u
     *   $ zypper dup
     *   ....upgrades to 13.2...
     * \endcode
     * The same can be achieved by using zyppers --releasever global option:
     * \code
     *   $ zypper --releasever 13.2 lr -u
     *   $ zypper --releasever 13.2 dup
     *   ....upgrades to 13.2...
     * \endcode
     * (see \ref zypp-envars)
     *
     * \code
     * Example:
     * ftp://user:secret@site.net/$arch/ -> ftp://user:secret@site.net/i686/
     * http://site.net/?basearch=$basearch -> http://site.net/?basearch=i386
     * \endcode
     */
    struct RepoVariablesStringReplacer : public std::unary_function<const std::string &, std::string>
    {
      std::string operator()( const std::string & value_r ) const;
    };

    /**
     * \short Functor replacing repository variables
     *
     * Replaces repository variables in the path and query part of the URL.
     * \see RepoVariablesStringReplacer
     */
    struct RepoVariablesUrlReplacer : public std::unary_function<const Url &, Url>
    {
      Url operator()( const Url & url_r ) const;
    };
  } // namespace repo
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoVariablesStringReplacer Helper managing repo variables replaced strings */
  typedef base::ValueTransform<std::string, repo::RepoVariablesStringReplacer> RepoVariablesReplacedString;

  /** \relates RepoVariablesStringReplacer Helper managing repo variables replaced string lists */
  typedef base::ContainerTransform<std::list<std::string>, repo::RepoVariablesStringReplacer> RepoVariablesReplacedStringList;

  /** \relates RepoVariablesUrlReplacer Helper managing repo variables replaced urls */
  typedef base::ValueTransform<Url, repo::RepoVariablesUrlReplacer> RepoVariablesReplacedUrl;

  /** \relates RepoVariablesUrlReplacer Helper managing repo variables replaced url lists */
  typedef base::ContainerTransform<std::list<Url>, repo::RepoVariablesUrlReplacer> RepoVariablesReplacedUrlList;

} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif
