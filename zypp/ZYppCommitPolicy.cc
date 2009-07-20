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

#include "zypp/base/String.h"

#include "zypp/ZConfig.h"

#include "zypp/ZYppCommitPolicy.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ZYppCommitPolicy::ZYppCommitPolicy()
      : _restrictToMedia    ( 0 )
      , _dryRun             ( false )
      , _rpmInstFlags       ( ZConfig::instance().rpmInstallFlags() )
      , _syncPoolAfterCommit( true )
  {}

  std::ostream & operator<<( std::ostream & str, const ZYppCommitPolicy & obj )
  {
    str << "CommitPolicy(";
    if ( obj.restrictToMedia() )
      str << " restrictToMedia:" << obj.restrictToMedia();
    if ( obj.dryRun() )
      str << " dryRun";
    if ( obj.syncPoolAfterCommit() )
      str << " syncPoolAfterCommit";
    if ( obj.rpmInstFlags() )
      str << " rpmInstFlags{" << str::hexstring(obj.rpmInstFlags()) << "}";
    return str << " )";
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
