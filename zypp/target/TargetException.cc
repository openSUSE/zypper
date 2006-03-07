/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/TargetException.cc
 *
*/

#include <string>
#include <iostream>

#include "zypp/target/TargetException.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace target {
  /////////////////////////////////////////////////////////////////

    std::ostream & TargetAbortedException::dumpOn( std::ostream & str ) const
    {
      return str << "Installation aborted by user" << endl;
    }


  /////////////////////////////////////////////////////////////////
  } // namespace target
} // namespace zypp
///////////////////////////////////////////////////////////////////
