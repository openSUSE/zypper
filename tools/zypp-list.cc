#define INCLUDE_TESTSETUP_WITHOUT_BOOST
#include "zypp/../tests/lib/TestSetup.h"
#undef  INCLUDE_TESTSETUP_WITHOUT_BOOST

#include <algorithm>
#include <zypp/PoolQuery.h>

static std::string appname( "zypp-list" );

#define message	cerr
#define OUT 	cout
using std::flush;

int errexit( const std::string & msg_r = std::string(), int exit_r = 100 )
{
  if ( ! msg_r.empty() )
  {
    cerr << endl << msg_r << endl << endl;
  }
  return exit_r;
}

int usage( const std::string & msg_r = std::string(), int exit_r = 100 )
{
  if ( ! msg_r.empty() )
  {
    cerr << endl << msg_r << endl << endl;
  }
  cerr << "Usage: " << appname << " [GLOBALOPTS] COMMAND" << endl;
  cerr << "List pool items according to command." << endl;
  cerr << endl;
  cerr << "GLOBALOPTS:" << endl;
  cerr << "  --root   Load repos from the system located below ROOTDIR. If ROOTDIR" << endl;
  cerr << "           denotes a sover testcase, the testcase is loaded." << endl;
  cerr << "  -i, --installed Process installed packages only." << endl;
  cerr << endl;
  cerr << "COMMANDS:" << endl;
  cerr << "  locks:   List all locked pool items." << endl;
  cerr << "    ." << endl;
  cerr << endl;
  return exit_r;
}

void startup( const Pathname & sysRoot = "/", bool onlyInstalled = false )
{
  ZConfig::instance();
  sat::Pool satpool( sat::Pool::instance() );

  if ( TestSetup::isTestcase( sysRoot ) )
  {
    message << str::form( "*** Load Testcase from '%s'", sysRoot.c_str() ) << endl;
    TestSetup test;
    test.loadTestcaseRepos( sysRoot );
    dumpRange( message, satpool.reposBegin(), satpool.reposEnd() ) << endl;
  }
  else if ( TestSetup::isTestSetup( sysRoot ) )
  {
    message << str::form( "*** Load TestSetup from '%s'", sysRoot.c_str() ) << endl;
    TestSetup test( sysRoot, Arch_x86_64 );
    test.loadRepos();
    dumpRange( message, satpool.reposBegin(), satpool.reposEnd() ) << endl;
  }
  else
  {
    // a system
    message << str::form( "*** Load system at '%s'", sysRoot.c_str() ) << endl;
    if ( true )
    {
      message << "*** load target '" << Repository::systemRepoAlias() << "'\t";
      getZYpp()->initializeTarget( sysRoot );
      getZYpp()->target()->load();
      message << satpool.systemRepo() << endl;
    }

    if ( !onlyInstalled )
    {
      RepoManager repoManager( sysRoot );
      RepoInfoList repos = repoManager.knownRepositories();
      for_( it, repos.begin(), repos.end() )
      {
        RepoInfo & nrepo( *it );

        if ( ! nrepo.enabled() )
          continue;

        if ( ! repoManager.isCached( nrepo ) )
        {
          message << str::form( "*** omit uncached repo '%s' (do 'zypper refresh')", nrepo.name().c_str() ) << endl;
          continue;
        }

        message << str::form( "*** load repo '%s'\t", nrepo.name().c_str() ) << flush;
        try
        {
          repoManager.loadFromCache( nrepo );
          message << satpool.reposFind( nrepo.alias() ) << endl;
        }
        catch ( const Exception & exp )
        {
          message << exp.asString() + "\n" + exp.historyAsString() << endl;
          message << str::form( "*** omit broken repo '%s' (do 'zypper refresh')", nrepo.name().c_str() ) << endl;
          continue;
        }
      }
    }
  }
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;
  appname = Pathname::basename( argv[0] );
  --argc,++argv;

  if ( ! argc )
  {
    return usage();
  }

  ///////////////////////////////////////////////////////////////////

  Pathname sysRoot( "/" );
  if ( argc && (*argv) == std::string("--root") )
  {
    --argc,++argv;
    if ( ! argc )
      return errexit("--root requires an argument.");

    if ( ! PathInfo( *argv ).isDir() )
      return errexit("--root requires a directory.");

    sysRoot = *argv;
    --argc,++argv;
  }

  bool onlyInstalled( false );
  if ( argc && (*argv) == std::string("--installed") )
  {
    --argc,++argv;
    onlyInstalled = true;
  }

  ///////////////////////////////////////////////////////////////////

  if ( ! argc )
  {
    return usage();
  }

  startup( sysRoot, onlyInstalled );
  ResPool pool( ResPool::instance() );

  if ( argc && (*argv) == std::string("locked") )
  {
    OUT << "*** Locked:" << endl;
    for_( it, pool.begin(), pool.end() )
    {
      if ( (*it).status().isLocked() )
	OUT << *it << endl;
    }
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}
