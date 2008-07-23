/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CapMatch.h
 *
*/
#ifndef ZYPP_CAPMATCH_H
#define ZYPP_CAPMATCH_H

#include <iosfwd>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : CapMatch
  //
  /** Tri state Capability match result.
   *
   * CapMatch::irrelevant denotes a result value that should be ignored.
   * Therfore it behaves neutral when used in <tt>! && ||</tt> expressions.
   *
   * \code
   *   CapMatch any
   *   (CapMatch::irrelevant && any) == any                  // true
   *   (CapMatch::irrelevant || any) == any                  // true
   *   ( !CapMatch::irrelevant )     == CapMatch::irrelevant // true
   * \endcode
  */
  class CapMatch
  {
    enum Result { NOMATCH, MATCH, IRRELEVANT };

  public:

    CapMatch()
    : _result( IRRELEVANT )
    {}

    CapMatch( bool val_r )
    : _result( val_r ? MATCH : NOMATCH )
    {}

    static const CapMatch yes;
    static const CapMatch no;
    static const CapMatch irrelevant;

    friend bool operator==( const CapMatch & lhs, const CapMatch & rhs )
    { return lhs._result == rhs._result; }

    friend bool operator!=( const CapMatch & lhs, const CapMatch & rhs )
    { return lhs._result != rhs._result; }

    friend CapMatch operator!( const CapMatch & lhs )
    {
      if ( lhs._result == CapMatch::IRRELEVANT )
        return lhs;
      return !(lhs._result == CapMatch::MATCH);
    }

    friend CapMatch operator&&( const CapMatch & lhs, const CapMatch & rhs )
    {
      if ( lhs._result == CapMatch::IRRELEVANT )
        return rhs;
      if ( rhs._result == CapMatch::IRRELEVANT )
        return lhs;
      return    (lhs._result == CapMatch::MATCH)
             && (rhs._result == CapMatch::MATCH);
    }

    friend CapMatch operator||( const CapMatch & lhs, const CapMatch & rhs )
    {
      if ( lhs._result == CapMatch::IRRELEVANT )
        return rhs;
      if ( rhs._result == CapMatch::IRRELEVANT )
        return lhs;
      return    (lhs._result == CapMatch::MATCH)
             || (rhs._result == CapMatch::MATCH);
    }

    friend std::ostream & operator<<( std::ostream & str, const CapMatch & obj );

  private:
    Result _result;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates CapMatch Stream output */
  std::ostream & operator<<( std::ostream & str, const CapMatch & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPMATCH_H
