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

    /////////////////////////////////////////////////////////////////
  } // namespace iostr
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
