/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp2/JustDownloadPackagesCommiter.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  JustDownloadPackagesCommiter::JustDownloadPackagesCommiter( const Pathname &path )
  {}
  
  JustDownloadPackagesCommiter::~JustDownloadPackagesCommiter()
  {}
    
  ZYppCommitResult JustDownloadPackagesCommiter::commit( ResPool pool_r, const ZYppCommitPolicy & policy_r )
  {}
  
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_COMMIT_IFACE_H
