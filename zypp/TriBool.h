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
#include <string>
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
   *
   * \warning Be carefull.esp. when comparing \ref TriBool using
   * \c operator==, as <b><tt>( indeterminate == indeterminate )</tt></b>
   * does \b not evaluate \b true. It's \c indeterminate.
   *
   * \see http://www.boost.org/doc/html/tribool.html
   * \ingroup BOOST
  */
  typedef boost::logic::tribool TriBool;
  using   boost::logic::tribool;
  using   boost::logic::indeterminate;

  inline std::string asString( const TriBool & val_r, const std::string & istr_r = std::string(),
						      const std::string & tstr_r = std::string(),
						      const std::string & fstr_r = std::string() )
  {
    std::string ret;
    if (indeterminate(val_r))
      ret = ( istr_r.empty() ? "indeterminate" : istr_r );
    else if (val_r)
      ret = ( tstr_r.empty() ? "true" : tstr_r );
    else
      ret = ( fstr_r.empty() ? "false" : fstr_r );
    return ret;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
namespace boost
{
    namespace logic
    {
      /** \relates TriBool stream output */
      inline std::ostream & operator<<(std::ostream & s, const tribool & obj)
      { return s << zypp::asString( obj ); }
    }
}
#endif // ZYPP_TRIBOOL_H
