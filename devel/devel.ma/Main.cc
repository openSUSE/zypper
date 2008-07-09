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

void mksrc( const std::string & url, const std::string & alias )
{
  RepoManager repoManager( makeRepoManager( sysRoot ) );

  RepoInfo nrepo;
  nrepo
      .setAlias( alias )
      .setName( alias )
      .setEnabled( true )
      .setAutorefresh( false )
      .addBaseUrl( Url(url) );

  if ( ! repoManager.isCached( nrepo ) )
  {
    repoManager.buildCache( nrepo );
  }

  repoManager.loadFromCache( nrepo );
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  ResPool   pool( ResPool::instance() );
  sat::Pool satpool( sat::Pool::instance() );

  mksrc( "file:///schnell/CD-ARCHIVE/SLES10/SLE-10-SP1/SLES-10-SP1-GM/ia64/DVD1", "SLE" );
  mksrc( "file:///mounts/dist/install/SLP/SLES-10-SP2-AS-LATEST/i386/CD1", "factorytest" );

  USR << "pool: " << pool << endl;

  getSel<Product>( "SUSE_SLES_SP1" )->setStatus( ui::S_Install );
  //getSel<Pattern>( "basesystem" )->setStatus( ui::S_Install );
  //getSel<Pattern>( "slesas-ofed-base" )->setStatus( ui::S_Install );

  if ( 1 )
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
