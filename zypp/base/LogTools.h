/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/LogTools.h
 *
*/
#ifndef ZYPP_BASE_LOGTOOLS_H
#define ZYPP_BASE_LOGTOOLS_H

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <set>
#include "zypp/base/Logger.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** Print range defined by iterators.
   * \code
   * intro [ pfx ITEM [ { sep ITEM }+ ] sfx ] extro
   * \endcode
   * The defaults print the range enclosed in \c {}, one item per
   * line indented by 2 spaces.
   * \code
   * {
   *   item1
   *   item2
   * }
   * {} // on empty rande
   * \endcode
   * A comma separated list enclosed in \c () would be
   * \code
   * dumpRange( stream, begin, end, "(", "", ", ", "", ")" );
   * \endcode
  */
  template<class _Iterator>
    std::ostream & dumpRange( std::ostream & str,
                              _Iterator begin, _Iterator end,
                              const std::string & intro = "{",
                              const std::string & pfx   = "\n  ",
                              const std::string & sep   = "\n  ",
                              const std::string & sfx   = "\n",
                              const std::string & extro = "}" )
    {
      str << intro;
      if ( begin != end )
        {
          str << pfx << *begin;
          for (  ++begin; begin != end; ++begin )
            str << sep << *begin;
          str << sfx;
        }
      return str << extro;
    }

  template<class _Iterator>
    std::ostream & dumpRangeLine( std::ostream & str,
                                  _Iterator begin, _Iterator end )
    { return dumpRange( str, begin, end, "(", "", ", ", "", ")" ); }

  template<class _Tp>
    std::ostream & operator<<( std::ostream & str, const std::vector<_Tp> & obj )
    { return dumpRange( str, obj.begin(), obj.end() ); }

  template<class _Tp>
    std::ostream & operator<<( std::ostream & str, const std::set<_Tp> & obj )
    { return dumpRange( str, obj.begin(), obj.end() ); }

  template<class _Tp>
    std::ostream & operator<<( std::ostream & str, const std::list<_Tp> & obj )
    { return dumpRange( str, obj.begin(), obj.end() ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_LOGTOOLS_H
