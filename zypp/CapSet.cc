/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CapSet.cc
 *
*/
#include <iostream>
#include <iterator>

#include "zypp/CapSet.h"
#include "zypp/Capability.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  bool CapOrder::operator()( const Capability & lhs, const Capability & rhs ) const
  {
    // fix
    return lhs.sayFriend() < rhs.sayFriend();
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const CapSet & obj )
  {
    copy( obj.begin(), obj.end(), ostream_iterator<Capability>(str,"\n") );
    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
