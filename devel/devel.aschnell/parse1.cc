
// g++ parse1.cc -o parse1 -O2 -Wall -lzypp


#include "zypp/base/Logger.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Exception.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/ResPoolProxy.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/Package.h"
#include "zypp/Pattern.h"
#include "zypp/Language.h"
#include "zypp/PackageKeyword.h"
#include "zypp/NameKindProxy.h"
#include "zypp/RepoManager.h"
#include "zypp/RepoInfo.h"


using namespace std;
using namespace zypp;


int
main (int argc, char* argv[])
{
    MIL << "===[START]==========================================" << endl;

    // TODO: tell RepoManager and SQLite to use different path

    RepoManager repoManager;
    RepoInfoList repos = repoManager.knownRepositories();

#if 1
    if ( repos.empty() )
    {
	RepoInfo nrepo;
	nrepo
	    .setAlias( "factorytest" )
	    .setName( "Test Repo for factory." )
	    .setEnabled( true )
	    .setAutorefresh( false )
	    .addBaseUrl(Url("ftp://dist.suse.de/install/stable-x86/"));
	    // .addBaseUrl(Url("http://software.opensuse.org/download/home:/Arvin42/openSUSE_Factory/"));
	    // .addBaseUrl(Url("file:///ARVIN/zypp/trunk/repotools/"));

	repoManager.addRepository( nrepo );
	repos = repoManager.knownRepositories();

	SEC << "refreshMetadata" << endl;
	repoManager.refreshMetadata( nrepo );
	SEC << "buildCache" << endl;
	repoManager.buildCache( nrepo );
    }
#endif

    ResPool pool( getZYpp()->pool() );

    USR << "pool: " << pool << endl;

    for ( RepoInfoList::iterator it = repos.begin(); it != repos.end(); it++ )
    {
	RepoInfo& nrepo( *it );

#if 1
	SEC << "refreshMetadata" << endl;
	repoManager.refreshMetadata( nrepo );
	SEC << "buildCache" << endl;
	repoManager.buildCache( nrepo );
#endif

	// here SQLite is upto-date

#if 1
	SEC << nrepo << endl;
	Repository nrep( repoManager.createFromCache( nrepo ) );
	const zypp::ResStore& store( nrep.resolvables() );
	getZYpp()->addResolvables( store );
#endif
    }

    USR << "pool: " << pool << endl;

    for (ResPool::const_iterator it = pool.begin(); it != pool.end(); it++)
    {
	USR << (*it)->name() << ' '
	    << (*it)->kind() << ' '
	    << (*it)->arch() << ' '
	    << (*it)->edition() << endl;
    }

    INT << "===[END]============================================" << endl;

    return 0;
}
