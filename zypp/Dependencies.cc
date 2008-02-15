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

#include "zypp/base/LogTools.h"

#include "zypp/Dependencies.h"

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
    if ( ! obj[Dep::PROVIDES].empty() )
      str << "PROVIDES:" << endl << obj[Dep::PROVIDES];
    if ( ! obj[Dep::PREREQUIRES].empty() )
      str << "PREREQUIRES:" << endl << obj[Dep::PREREQUIRES];
    if ( ! obj[Dep::REQUIRES].empty() )
      str << "REQUIRES:" << endl << obj[Dep::REQUIRES];
    if ( ! obj[Dep::CONFLICTS].empty() )
      str << "CONFLICTS:" << endl << obj[Dep::CONFLICTS];
    if ( ! obj[Dep::OBSOLETES].empty() )
      str << "OBSOLETES:" << endl << obj[Dep::OBSOLETES];
    if ( ! obj[Dep::RECOMMENDS].empty() )
      str << "RECOMMENDS:" << endl << obj[Dep::RECOMMENDS];
    if ( ! obj[Dep::SUGGESTS].empty() )
      str << "SUGGESTS:" << endl << obj[Dep::SUGGESTS];
    if ( ! obj[Dep::SUPPLEMENTS].empty() )
      str << "SUPPLEMENTS:" << endl << obj[Dep::SUPPLEMENTS];
    if ( ! obj[Dep::ENHANCES].empty() )
      str << "ENHANCES:" << endl << obj[Dep::ENHANCES];
    if ( ! obj[Dep::FRESHENS].empty() )
      str << "FRESHENS:" << endl << obj[Dep::FRESHENS];
    return str << "]";
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
