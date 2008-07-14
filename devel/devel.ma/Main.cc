#include "Tools.h"

#include "zypp/base/Easy.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/TmpPath.h"

#include "zypp/RepoManager.h"
#include "zypp/RepoInfo.h"
#include "zypp/repo/PackageProvider.h"

#include "zypp/ResPoolProxy.h"

using std::endl;
using namespace zypp;

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( getenv("SYSROOT") ? getenv("SYSROOT") : "/Local/ROOT" );

///////////////////////////////////////////////////////////////////

bool solve()
{
  bool rres = false;
  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    rres = getZYpp()->resolver()->resolvePool();
  }
  if ( ! rres )
  {
    ERR << "resolve " << rres << endl;
    return false;
  }
  MIL << "resolve " << rres << endl;
  return true;
}

ManagedFile providePkg( const PoolItem & pi )
{
  Package::constPtr p = asKind<Package>( pi.resolvable() );
  if ( ! pi )
    return ManagedFile();

  repo::RepoMediaAccess access;
  std::list<Repository> repos;
  repo::DeltaCandidates deltas( repos );
  repo::PackageProvider pkgProvider( access, p, deltas );

  return pkgProvider.providePackage();
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  RepoManager repoManager( makeRepoManager( sysRoot ) );
  ResPool     pool( ResPool::instance() );
  sat::Pool   satpool( sat::Pool::instance() );

//   mksrc( "file:///schnell/CD-ARCHIVE/SLES10/SLE-10-SP1/SLES-10-SP1-GM/ia64/DVD1", "SLE" );
//   mksrc( "file:///mounts/dist/install/SLP/SLES-10-SP2-AS-LATEST/i386/CD1", "factorytest" );
//   mksrc( "iso:///?iso=openSUSE-10.3-GM-DVD9-BiArch-DVD1.iso&url=file:///schnell/CD-ARCHIVE/10.3/iso", "10.3.iso", repoManager );
  mksrc( "file:///Local/SRC/test/", "test", repoManager );

  USR << "pool: " << pool << endl;
  PoolItem pi ( getPi<Package>("BitTorrent-curses", Edition("4.0.3-115"), Arch_i586 ) );
  MIL << pi << endl;
  if ( pi )
  {
    providePkg( pi );
  }

  //getSel<Pattern>( "basesystem" )->setStatus( ui::S_Install );
  //getSel<Pattern>( "slesas-ofed-base" )->setStatus( ui::S_Install );

  if ( 0 )
  {
    vdumpPoolStats( USR << "Transacting:"<< endl,
                    make_filter_begin<resfilter::ByTransact>(pool),
                    make_filter_end<resfilter::ByTransact>(pool) ) << endl;
    solve();
    vdumpPoolStats( USR << "Transacting:"<< endl,
                    make_filter_begin<resfilter::ByTransact>(pool),
                    make_filter_end<resfilter::ByTransact>(pool) ) << endl;
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}
