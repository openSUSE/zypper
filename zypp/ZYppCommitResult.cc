/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZYppCommitResult.cc
 *
*/

#include <iostream>

#include "zypp/ZYppCommitResult.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  std::ostream & operator<<( std::ostream & str, const ZYppCommitResult & obj )
  {
    str << "CommitResult " << obj._result
        << " (errors " << obj._errors.size()
        << ", remaining " << obj._remaining.size()
        << ", srcremaining " << obj._srcremaining.size()
        << ")";
    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
