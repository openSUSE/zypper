/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/NVRA.cc
 *
*/
#include <iostream>

#include "zypp/NVRA.h"
#include "zypp/Resolvable.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  NVRA::NVRA( ResTraits< Resolvable >::constPtrType res_r )
  {
    if ( res_r )
      {
        *this = NVRA( res_r->name(), res_r->edition(), res_r->arch() );
      }
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const NVRA & obj )
  {
    return str << obj.name << '-' << obj.edition << '.' << obj.arch;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
