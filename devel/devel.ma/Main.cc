#include <iostream>

#include "zypp/base/Easy.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/InputStream.h"

#include "zypp/RepoManager.h"

using std::endl;
using namespace zypp;

///////////////////////////////////////////////////////////////////

RepoManager makeRepoManager( const Pathname & mgrdir_r )
{
  RepoManagerOptions mgropt;

  mgropt.repoCachePath    = mgrdir_r/"cache";
  mgropt.repoRawCachePath = mgrdir_r/"raw_cache";
  mgropt.knownReposPath   = mgrdir_r/"repos";

  return RepoManager( mgropt );
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  RepoManager repoManager( makeRepoManager( "/tmp/myrepos" ) );
  RepoInfoList repos = repoManager.knownRepositories();
  SEC << repos << endl;

  if ( repos.empty() )
  {
    RepoInfo nrepo;
    nrepo
	.setAlias( "factorytest" )
	.setName( "Test Repo for factory." )
	.setEnabled( true )
	.setAutorefresh( false )
	.addBaseUrl( Url("ftp://dist.suse.de/install/stable-x86/") );

    repoManager.addRepository( nrepo );
    repos = repoManager.knownRepositories();
    SEC << repos << endl;

//    SEC << "refreshMetadat" << endl;
//    repoManager.refreshMetadata( nrepo );
//    SEC << "buildCache" << endl;
//    repoManager.buildCache( nrepo );
//    SEC << "------" << endl;
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}
