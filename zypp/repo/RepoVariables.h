/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/RepoVariables.h
 */
#ifndef ZYPP_REPO_VARIABLES_H_
#define ZYPP_REPO_VARIABLES_H_

#include <string>
#include "zypp/base/Function.h"
#include "zypp/base/ValueTransform.h"
#include "zypp/Url.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace repo
  {
    ///////////////////////////////////////////////////////////////////
    /// \class RepoVarExpand
    /// \brief Functor expanding repo variables in a string
    ///
    /// Known variables are determined by a callback function taking a variable
    /// name and returning a pointer to the variable value or \c nullptr if unset.
    ///
    /// The \c $ character introduces variable expansion. A valid variable name
    /// is any non-empty case-insensitive sequence of <tt>[[:alnum:]_]</tt>.
    /// The variable name to be expanded may be enclosed in braces, which are
    /// optional but serve to protect the variable to be expanded from characters
    /// immediately following it which could be interpreted as part of the name.
    ///
    /// When braces are used, the matching ending brace is the first \c } not
    /// escaped by a backslash and not within an embedded variable expansion.
    /// Within braces only \c $, \c } and \c backslash are escaped by a
    /// backslash. There is no escaping outside braces, to stay comaptible
    /// with \c YUM (which does not support braces).
    ///
    /// <ul>
    /// <li> \c ${variable}
    /// If \c variable is unset the original is preserved like in \c YUM.
    /// Otherwise, the value of \c variable is substituted.</li>
    ///
    /// <li> \c ${variable:-word} (default value)
    /// If \c variable is unset or empty, the expansion of \c word is substituted.
    /// Otherwise, the value of \c variable is substituted.</li>
    ///
    /// <li> \c ${variable:+word} (alternate value)
    /// If variable is unset or empty nothing is substituted.
    /// Otherwise, the expansion of \c word is substituted.</li>
    /// </ul>
    struct RepoVarExpand
    {
      /** Function taking a variable name and returning a pointer to the variable value or \c nullptr if unset. */
      typedef function<const std::string * ( const std::string & )> VarRetriever;

      /** Return a copy of \a value_r with embedded variables expanded. */
      std::string operator()( const std::string & value_r, VarRetriever varRetriever_r ) const;
#ifndef SWIG // Swig treats it as syntax error
      /** \overload moving */
      std::string operator()( std::string && value_r, VarRetriever varRetriever_r ) const;
#endif
    };

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
