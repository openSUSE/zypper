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
//#include "zypp/base/Logger.h"

#include "zypp/base/IOStream.h"

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

    /////////////////////////////////////////////////////////////////
  } // namespace iostr
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
