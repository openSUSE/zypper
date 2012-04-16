/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/IOStream.cc
 *
*/
#include <iostream>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/int.hpp>
//#include "zypp/base/Logger.h"

#include "zypp/base/IOStream.h"
#include "zypp/base/String.h"

using std::endl;
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace iostr
  { /////////////////////////////////////////////////////////////////

    /******************************************************************
     **
     **	FUNCTION NAME : getline
     **	FUNCTION TYPE : std::string
     */
    std::string getline( std::istream & str )
    {
      static const unsigned tmpBuffLen = 1024;
      static char           tmpBuff[tmpBuffLen];
      std::string ret;
      do {
        str.clear();
        str.getline( tmpBuff, tmpBuffLen ); // always writes '\0' terminated
        ret += tmpBuff;
      } while( str.rdstate() == std::ios::failbit );

      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : EachLine
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : EachLine::EachLine
    //	METHOD TYPE : Ctor
    //
    EachLine::EachLine( std::istream & str_r, unsigned lineNo_r )
      : _str( str_r )
      , _lineStart( -1 )
      , _lineNo( lineNo_r )
      , _valid( true )
    {
      next();
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : EachLine::next
    //	METHOD TYPE : bool
    //
    bool EachLine::next()
    {
      if ( ! _valid )
      {
	return false;
      }

      if ( ! _str ) // usg: saw EOF in previous read
      {
	_line.clear();
	return(_valid = false);
      }

      _lineStart = _str.tellg();
      _line = iostr::getline( _str );
      ++_lineNo;
      if ( _str.fail() || _str.bad() )
      {
	_line.clear();
	return(_valid = false);
      }
      return(_valid = true);
    }

    ///////////////////////////////////////////////////////////////////
    // forEachLine
    ///////////////////////////////////////////////////////////////////

    int forEachLine( std::istream & str_r, function<bool(int, std::string)> consume_r )
    {
      int lineno = 0;
      while ( str_r )
      {
	std::string line( getline( str_r ) );
	if ( ! (str_r.fail() || str_r.bad()) )
	{
	  // line contains valid data to be consumed.
	  ++lineno;
	  if ( consume_r && ! consume_r( lineno, line ) )
	  {
	    lineno = -lineno;
	    break;
	  }
	}
      }
      return lineno;
    }

    // MPL checks to assert equal values for PF_?TRIM and str::?TRIM
    BOOST_MPL_ASSERT_RELATION( int(PF_LTRIM), ==, int(str::L_TRIM) );
    BOOST_MPL_ASSERT_RELATION( int(PF_RTRIM), ==, int(str::R_TRIM) );

    int simpleParseFile( std::istream & str_r, ParseFlags flags_r, function<bool(int, std::string)> consume_r )
    {
      return forEachLine( str_r,
			  [&]( int num_r, std::string line_r )->bool
			  {
			    if ( ! consume_r )
			      return true;

			    if ( flags_r )
			    {
			      if ( flags_r & PF_TRIM )
				line_r = str::trim( line_r, str::Trim( unsigned(flags_r & PF_TRIM) ) );

			      if ( flags_r & ~PF_TRIM )
			      {
				const char* firstNW = line_r.c_str();
				while ( *firstNW == ' ' || *firstNW == '\t' )
				  ++firstNW;
				switch ( *firstNW )
				{
				  case '\0':	if ( flags_r & PF_SKIP_EMPTY )		return true; break;
				  case '#':	if ( flags_r & PF_SKIP_SHARP_COMMENT )	return true; break;
				}
			      }
			    }
			    return consume_r( num_r, line_r );
			  } );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace iostr
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
