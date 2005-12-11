/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/IOStream.h
 *
*/
#ifndef ZYPP_BASE_IOSTREAM_H
#define ZYPP_BASE_IOSTREAM_H

#include <iosfwd>
#include <boost/io/ios_state.hpp>

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  /** Iostream related utilities.
  */
  namespace iostr
  { /////////////////////////////////////////////////////////////////

    /** Save and restore streams \c width, \c precision
     * and \c fmtflags.
    */
    typedef boost::io::ios_base_all_saver IosFmtFlagsSaver;


    /** Read one line from stream.
     *
     * Reads everything up to the next newline or EOF. newline
     * is read but not returned.
     *
     * \see \ref forEachLine
    */
    std::string getline( std::istream & str );


    /** Simple lineparser: Call functor \a consume_r for each line.
     *
     * \param str_r The istream to read from.
     * \param consume_r A reference to a function or functor.
     * \code
     * void consume( const std::string & )
     * { ... }
     *
     * struct Consume : public std::unary_function<const std::string &, void>
     * {
     *   void operator()( const std::string & line_r )
     *   { ... }
     * };
     * \endcode
     *
     * \return A reference to \a consume_r.
    */
    template<class _Function>
      _Function & forEachLine( std::istream & str_r, _Function & consume_r )
      {
        while ( str_r )
          {
            std::string l = getline( str_r );
            if ( ! (str_r.fail() || str_r.bad()) )
              {
                // l contains valid data to be consumed.
                consume_r( l );
              }
          }
        return consume_r;
      }

    /////////////////////////////////////////////////////////////////
  } // namespace iostr
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_IOSTREAM_H
