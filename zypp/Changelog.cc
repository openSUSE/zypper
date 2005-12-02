/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Changelog.cc
 *
*/
#include <iostream>
#include <set>

#include "zypp/Changelog.h"

using namespace std;

namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** \relates ChangelogEntry */
  std::ostream & operator<<( std::ostream & out, const ChangelogEntry & obj )
  { 
    out << obj.date() << " " << obj.author() << endl << obj.text() << endl;
    return out;
  }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
