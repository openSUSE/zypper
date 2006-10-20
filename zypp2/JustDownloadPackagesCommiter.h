/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp2/JustDownloadPackagesCommiter.h
 *
*/
#ifndef ZYPP_JustDownloadPackagesCommiter_H
#define ZYPP_JustDownloadPackagesCommiter_H

#include <iosfwd>
#include <map>
#include <list>
#include <set>
#include <string>

#include "zypp/Pathname.h"
#include "zypp/ZYppCommit.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class JustDownloadPackagesCommiter : public CommiterIface
  {
  public:
    JustDownloadPackagesCommiter( const Pathname & );
    virtual ~JustDownloadPackagesCommiter();
    
    virtual ZYppCommitResult commit( ResPool pool_r, const ZYppCommitPolicy & policy_r );
  private:
    Pathname _path;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_COMMIT_IFACE_H
