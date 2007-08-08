
// g++ parse2.cc -o parse2 -O2 -Wall -lzypp


#include "zypp/base/Logger.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Exception.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/ResPoolProxy.h"
#include "zypp/ResPool.h"
#include "zypp/ResPoolManager.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/Package.h"
#include "zypp/Pattern.h"
#include "zypp/Language.h"
#include "zypp/PackageKeyword.h"
#include "zypp/NameKindProxy.h"
#include "zypp/RepoManager.h"
#include "zypp/RepoInfo.h"
#include "zypp/TmpPath.h"


using namespace std;
using namespace zypp;
using namespace zypp::filesystem;


int
main (int argc, char* argv[])
{
    TmpDir tmpCachePath;
    TmpDir tmpRawCachePath;
    TmpDir tmpKnownReposPath;

    RepoManagerOptions opts;
    opts.repoCachePath = tmpCachePath.path();
    opts.repoRawCachePath = tmpRawCachePath.path();
    opts.knownReposPath = tmpKnownReposPath.path();

    RepoManager repoManager (opts);

    RepoInfo nrepo;
    nrepo.setAlias( "factorytest" )
	.setName( "Test Repo for Factory." )
	.setEnabled( true )
	.setAutorefresh( false )
	// .addBaseUrl(Url("ftp://dist.suse.de/install/stable-x86/"));
	// .addBaseUrl(Url("http://software.opensuse.org/download/home:/Arvin42/openSUSE_Factory/"));
	.addBaseUrl(Url("file:///ARVIN/zypp/trunk/repotools/tmp"));

    repoManager.addRepository( nrepo );

    ResPoolManager resPoolManager;

    RepoInfoList repos = repoManager.knownRepositories();
    for ( RepoInfoList::iterator it = repos.begin(); it != repos.end(); it++ )
    {
	RepoInfo& nrepo( *it );

	SEC << "refreshMetadata" << endl;
	repoManager.refreshMetadata( nrepo );
	SEC << "buildCache" << endl;
	repoManager.buildCache( nrepo );

	// here SQLite is upto-date

	SEC << nrepo << endl;
	Repository nrep = repoManager.createFromCache( nrepo );
	resPoolManager.insert(nrep.resolvables());
    }

    ResPool pool(resPoolManager.accessor());

    USR << "pool: " << pool << endl;

    for (ResPool::const_iterator itRes = pool.begin(); itRes != pool.end(); itRes++)
    {
	cout << (*itRes)->name() << ' '
	     << (*itRes)->kind() << ' '
	     << (*itRes)->arch() << ' '
	     << (*itRes)->edition() << endl;

	CapSet caps;

	caps = (*itRes)->dep(Dep::PROVIDES);
	for (CapSet::const_iterator itCap = caps.begin(); itCap != caps.end(); itCap++)
	    cout << "  Provides: " << itCap->asString() << std::endl;

	caps = (*itRes)->dep(Dep::REQUIRES);
	for (CapSet::const_iterator itCap = caps.begin(); itCap != caps.end(); itCap++)
	    cout << "  Requires: " << itCap->asString() << std::endl;

	cout << std::endl;
    }

    return 0;
}
