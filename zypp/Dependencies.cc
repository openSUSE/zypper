/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Dependencies.cc
 *
*/
#include <iostream>

#include "zypp/Dependencies.h"
#include "zypp/CapSet.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const Dependencies & obj )
  {
    str << "Dependencies: [" << endl;
    if ( ! obj.provides.empty() )
      str << "PROVIDES:" << endl << obj.provides;
    if ( ! obj.prerequires.empty() )
      str << "PREREQUIRES:" << endl << obj.prerequires;
    if ( ! obj.requires.empty() )
      str << "REQUIRES:" << endl << obj.requires;
    if ( ! obj.conflicts.empty() )
      str << "CONFLICTS:" << endl << obj.conflicts;
    if ( ! obj.obsoletes.empty() )
      str << "OBSOLETES:" << endl << obj.obsoletes;
    if ( ! obj.recommends.empty() )
      str << "RECOMMENDS:" << endl << obj.recommends;
    if ( ! obj.suggests.empty() )
      str << "SUGGESTS:" << endl << obj.suggests;
    if ( ! obj.freshens.empty() )
      str << "FRESHENS:" << endl << obj.freshens;
    return str << "]";
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
