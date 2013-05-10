#include "Tools.h"

#include <zypp/base/PtrTypes.h>
#include <zypp/base/Exception.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/ProvideNumericId.h>
#include <zypp/AutoDispose.h>

#include "zypp/ZYppFactory.h"
#include "zypp/ResPoolProxy.h"

#include "zypp/ZYppCallbacks.h"
#include "zypp/NVRAD.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/Package.h"
#include "zypp/Pattern.h"
#include "zypp/Language.h"
#include "zypp/Digest.h"
#include "zypp/PackageKeyword.h"


#include "zypp/parser/TagParser.h"
#include "zypp/parser/susetags/PackagesFileReader.h"
#include "zypp/parser/susetags/PackagesLangFileReader.h"
#include "zypp/parser/susetags/PatternFileReader.h"
#include "zypp/parser/susetags/ContentFileReader.h"
#include "zypp/parser/susetags/RepoIndex.h"
#include "zypp/parser/susetags/RepoParser.h"
#include "zypp/cache/CacheStore.h"
#include "zypp/RepoManager.h"
#include "zypp/RepoInfo.h"

#include "zypp/ui/PatchContents.h"
#include "zypp/ResPoolProxy.h"

using namespace std;
using namespace zypp;
using namespace zypp::functor;
using namespace zypp::ui;
using zypp::parser::TagParser;

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( "/Local/GTEST" );

///////////////////////////////////////////////////////////////////

struct Xprint
{
  bool operator()( const PoolItem & obj_r )
  {
//     handle( asKind<Package>( obj_r ) );
//     handle( asKind<Patch>( obj_r ) );
//     handle( asKind<Pattern>( obj_r ) );
//     handle( asKind<Product>( obj_r ) );
    return true;
  }

  void handle( const Package_constPtr & p )
  {
    if ( !p )
      return;
  }

  void handle( const Patch_constPtr & p )
  {
    if ( !p )
      return;
  }

  void handle( const Pattern_constPtr & p )
  {
    if ( !p )
      return;
  }

  void handle( const Product_constPtr & p )
  {
    if ( !p )
      return;
  }
};

///////////////////////////////////////////////////////////////////
/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "log.restrict" );
  INT << "===[START]==========================================" << endl;

  setenv( "ZYPP_CONF", (sysRoot/"zypp.conf").c_str(), 1 );

  RepoManager repoManager( makeRepoManager( sysRoot ) );
  RepoInfoList repos = repoManager.knownRepositories();
  SEC << "knownRepositories " << repos << endl;

  if ( repos.empty() )
  {
    RepoInfo nrepo;
    nrepo
	.setAlias( "factorytest" )
	.setName( "Test Repo for factory." )
	.setEnabled( true )
	.setAutorefresh( false )
	.addBaseUrl( Url("http://dist.suse.de/install/stable-x86/") );

    repoManager.addRepository( nrepo );
    repos = repoManager.knownRepositories();
  }

  for ( RepoInfoList::iterator it = repos.begin(); it != repos.end(); ++it )
  {
    RepoInfo & nrepo( *it );
    if ( ! nrepo.enabled() )
      continue;

    SEC << "refreshMetadata" << endl;
    repoManager.refreshMetadata( nrepo );

    if ( ! repoManager.isCached( nrepo ) || 0 )
    {
      if ( repoManager.isCached( nrepo ) )
      {
	SEC << "cleanCache" << endl;
	repoManager.cleanCache( nrepo );
      }
      SEC << "refreshMetadata" << endl;
      repoManager.refreshMetadata( nrepo, RepoManager::RefreshForced );
      SEC << "buildCache" << endl;
      repoManager.buildCache( nrepo );
    }

    SEC << nrepo << endl;
    Repository nrep( repoManager.createFromCache( nrepo ) );
    const zypp::ResStore & store( nrep.resolvables() );
    dumpPoolStats( SEC << "Store: " << endl,
		   store.begin(), store.end() ) << endl;
    getZYpp()->addResolvables( store );
  }

  ResPool pool( getZYpp()->pool() );
  vdumpPoolStats( USR << "Initial pool:" << endl,
		  pool.begin(),
		  pool.end() ) << endl;

  if ( 0 )
  {
    {
      //zypp::base::LogControl::TmpLineWriter shutUp;
      getZYpp()->initTarget( sysRoot );
    }
    MIL << "Added target: " << pool << endl;
  }

  std::for_each( pool.begin(), pool.end(), Xprint() );

 ///////////////////////////////////////////////////////////////////
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}
