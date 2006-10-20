/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp2/CommitIface.h
 *
*/
#ifndef ZYPP_COMMIT_IFACE_H
#define ZYPP_COMMIT_IFACE_H

#include <iosfwd>
#include <map>
#include <list>
#include <set>
#include <string>

#include "zypp/ZYppCommit.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class CommitIface
  {
  public:
    CommitIface() {};
    virtual ~CommitIface() {};
    virtual ZYppCommitResult commit( ResPool pool_r, const ZYppCommitPolicy & policy_r ) = 0;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_COMMIT_IFACE_H
