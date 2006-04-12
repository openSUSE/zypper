/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZYppCommitPolicy.cc
 *
*/

#include <iostream>

#include "zypp/ZYppCommitPolicy.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  std::ostream & operator<<( std::ostream & str, const ZYppCommitPolicy & obj )
  {
    str << "CommitPolicy(";
    if ( obj.restrictToMedia() )
      str << " restrictToMedia:" << obj.restrictToMedia();
    if ( obj.dryRun() )
      str << " dryRun";
    if ( obj.rpmNoSignature() )
      str << " rpmNoSignature";
    return str << " )";
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
