//
// g++ -Wall -std=c++11 zypp-install.cc -l zypp -o zypp-install
//
// A small (and simple) demo which walks through zypp, initializing
// and refreshing the repos, selecting packages ('zypper dup'),
// resolving dependencies and finally comitting/installing the
// result (in dry-run mode).
//
// No callbacks, questions or fancy output during commit, but it will
// do a 'zypper dup' if you'd remove the DryRun and DownloadOnly flag.
//
// So be careful if running it as root.
//
#include <iostream>

#define TEST_DEBUGLOG 0

#if ( TEST_DEBUGLOG )
#include <zypp/base/LogControl.h>
#endif
#include <zypp/ZYppFactory.h>
#include <zypp/RepoManager.h>
#include <zypp/ResPool.h>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using namespace zypp;

////////////////////////////////////////////////////////////////////////////////
int main( int argc, char * argv[] )
try {
  --argc;
  ++argv;
#if ( TEST_DEBUGLOG )
#warning debug log is on
  base::LogControl::instance().logfile( "/tmp/zypp-install.log" );
#endif

  Pathname sysRoot( "/" );
  ZYpp::Ptr zypp = getZYpp();		// acquire initial zypp lock

  ////////////////////////////////////////////////////////////////////////////////
  // init Target:
  {
    cout << "Initialize target at " << sysRoot << endl;
    zypp->initializeTarget( sysRoot );	// initialize target
    cout << "Loading target resolvables" << endl;
    zypp->getTarget()->load();		// load installed packages to pool
  }

  ////////////////////////////////////////////////////////////////////////////////
  // init Repos:
  {
    RepoManager repoManager( sysRoot );

    // sync the current repo set
    for ( RepoInfo & nrepo : repoManager.knownRepositories() )
    {
      if ( ! nrepo.enabled() )
	continue;

      // Often volatile media are sipped in automated environments
      // to avoid media chagne requests:
      if ( nrepo.url().schemeIsVolatile() )
	continue;

      bool refreshNeeded = false;
      if ( nrepo.autorefresh() )	// test whether to autorefresh repo metadata
      {
	for ( const Url & url : nrepo.baseUrls() )
	{
	  try
	  {
	    if ( repoManager.checkIfToRefreshMetadata( nrepo, url ) == RepoManager::REFRESH_NEEDED )
	    {
	      cout << "Need to autorefresh repo " << nrepo.alias() << endl;
	      refreshNeeded = true;
	    }
	    break;	// exit after first successful checkIfToRefreshMetadata
	  }
	  catch ( const Exception & exp )
	  {}	// Url failed, try next one...
	}
	// If all urls failed we can leave it to the code below to
	// fail if access is actually needed and still failing.
	// (missing metadata, package download, ...)
      }

      // initial metadata download or cache refresh
      if ( ! repoManager.isCached( nrepo ) || refreshNeeded )
      {
	cout << "Refreshing repo " << nrepo << endl;
	if ( repoManager.isCached( nrepo ) )
	{
	  repoManager.cleanCache( nrepo );
	}
	repoManager.refreshMetadata( nrepo );
	repoManager.buildCache( nrepo );
      }

      // load cache
      try
      {
	cout << "Loading resolvables from " << nrepo.alias() << endl;
	repoManager.loadFromCache( nrepo );// load available packages to pool
      }
      catch ( const Exception & exp )
      {
	// cachefile has old fomat (or is corrupted): try yo rebuild it
	repoManager.cleanCache( nrepo );
	repoManager.buildCache( nrepo );
	repoManager.loadFromCache( nrepo );
      }
    }
  }

  cout << zypp->pool() << endl;
  cout << "=====[pool ready]==============================" << endl;

  ////////////////////////////////////////////////////////////////////////////////
  // GO...
  ////////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////////
  // Select package to install...
  // For demo purpose do 'zypper dup'
  // otherwise select manually whatever you need...
  zypp->resolver()->doUpgrade();


  ////////////////////////////////////////////////////////////////////////////////
  // solve selection...
  {
    cout << "Solving dependencies..." << endl;

    unsigned attempt = 0;
    while ( ! zypp->resolver()->resolvePool() )
    {
      ++attempt;
      cout << "Solving dependencies: " << attempt << ". attempt failed" << endl;
      const ResolverProblemList & problems( zypp->resolver()->problems() );
      cout << problems.size() << " problems found..." << endl;
      // Problem:
      // ==============================
      // kdepim3-3.5.10-29.1.4.x86_64 requires libsasl2.so.2()(64bit), but this requirement
      // cannot be provided deleted providers: cyrus-sasl-2.1.25-28.1.2.x86_64
      // ------------------------------
      // Solution:
      // keep obsolete cyrus-sasl-2.1.25-28.1.2.x86_64
      // Solution:
      // remove lock to allow removal of kdepim3-3.5.10-29.1.4.x86_64
      // Solution:
      // remove lock to allow removal of kdepim3-3.5.10-29.1.4.x86_64
      // Solution:
      // break kdepim3-3.5.10-29.1.4.x86_64 by ignoring some of its dependencies

      ProblemSolutionList totry;	// only needed if you (interactively) resolve problems...

      unsigned probNo = 0;
      for ( const auto & probPtr : problems )
      {
	cout << "Problem " << ++probNo << ": " << probPtr->description() << endl;

	const ProblemSolutionList & solutions = probPtr->solutions();
	unsigned solNo = 0;
	for ( const auto & solPtr : solutions )
	{
	  cout << "  Solution " << ++solNo << ": " << solPtr->description() << endl;
	}

	// if you (interactively) resolve problems pick 1 solution per problem
	// and store it int the totry list. After having applied the selected
	// start a new attempt.
	//
	// It's not necessary to process all problems. You can pick a solution
	// for the first problem and retry immediately. Often one solution actually
	// resolves more than one reported problem.
	//
	// totry.push_back( solPtr );
      }


      if ( ! totry.empty() )
      {
	cout << "Apply selected solutions..." << endl;
	zypp->resolver()->applySolutions( totry );
	cout << "Solving dependencies..." << endl;
	continue;
      }
      // otherwise give up
      throw "Solving dependencies failed: Giving up!";
    }
    cout << "Dependencies solved" << endl;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // printing some stats...
  if ( false )
  {
    cout << "PoolItem summary (individual packages):" << endl;
    for ( const PoolItem & pi : zypp->pool() )
    {
      if ( pi.status().transacts() )
	cout << "  " << pi << endl;
    }
  }
  else
  {
    cout << "Selectable summary (grouped by name):" << endl;
    for ( const ui::Selectable_Ptr & sel : zypp->pool().proxy() )
    {
      if ( sel->toModify() )
	cout << "  " << sel << endl;
    }
  }

  ////////////////////////////////////////////////////////////////////////////////
  // finally commit..
  {
    cout << "Going to commit..." << endl;
    // dryRun and DownloadOnly will cause commit to skip
    // transaction steps, so you want to check for 'noError'
    // rather than 'allDone'.
    bool dryRunEtc = false;

    ZYppCommitPolicy policy;
    if ( true )
    {
      policy.dryRun( true );
      dryRunEtc = true;
    }
    if ( true  )
    {
      policy.downloadMode( DownloadOnly );
      dryRunEtc = true;
    }

    try
    {
      ZYppCommitResult result = zypp->commit( policy );	// go....
      if ( ! ( result.allDone() || ( dryRunEtc && result.noError() ) ) )
      {
	throw "Incomplete commit!";
	// ZYppCommitResult offers access to the TransactionStepList
	// where you can see which packages have been processed and
	// which not.
      }
      cout << "Commit succeeded" << endl;
    }
    catch ( const Exception & exp )
    {
      cout << "Commit aborted with exception:" << endl;
      throw;
    }
  }
  cout << "[bye]: " << endl;
  return 0;
}
catch ( const Exception & exp )
{ cerr << exp << endl << exp.historyAsString();	exit( 91 ); }
catch ( const std::exception & exp )
{ cerr << exp.what() << endl;			exit( 92 ); }
catch ( const char * exp )
{ cerr << (exp?exp:"Oops!") << endl;		exit( 93 ); }
catch (...)
{ cerr << "Oops!" << endl;			exit( 94 ); }


