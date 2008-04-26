/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/TriBool.h
 *
*/
#ifndef ZYPP_TRIBOOL_H
#define ZYPP_TRIBOOL_H

#include <iosfwd>
#include <boost/logic/tribool.hpp>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** 3-state boolean logic (\c true, \c false and \c indeterminate).
   * \code
   * namespace zypp
   * {
   *   typedef boost::logic::tribool TriBool;
   *   using   boost::logic::tribool;
   *   using   boost::logic::indeterminate;
   * }
   * \endcode
   * \see http://www.boost.org/doc/html/tribool.html
   * \ingroup BOOST
  */
  typedef boost::logic::tribool TriBool;
  using   boost::logic::tribool;
  using   boost::logic::indeterminate;

  /** \relates TriBool stream output */
  inline std::ostream & operator<<(std::ostream & s, const TriBool & obj)
  {
    if (obj == indeterminate)
      s << "indeterminate";
    else if (obj)
      s << "true";
    else
      s << "false";
    return s;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TRIBOOL_H
