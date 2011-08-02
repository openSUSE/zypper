#define INCLUDE_TESTSETUP_WITHOUT_BOOST
#include "zypp/../tests/lib/TestSetup.h"
#undef  INCLUDE_TESTSETUP_WITHOUT_BOOST

#include <algorithm>
#include <zypp/PoolQuery.h>
#include <zypp/ResObjects.h>

static std::string appname( "DumpSelectable" );

#define message cout
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
  cerr << "Usage: " << appname << " [--root ROOTDIR] [OPTIONS] NAME..." << endl;
  cerr << "  Load all enabled repositories (no refresh) and search for" << endl;
  cerr << "  Selectables names NAME" << endl;
  cerr << "  --root   Load repos from the system located below ROOTDIR. If ROOTDIR" << endl;
  cerr << "           denotes a sover testcase, the testcase is loaded." << endl;
  cerr << "  -v       Verbose list solvables data." << endl;
  cerr << "" << endl;
  return exit_r;
}

void dumpPi( std::ostream & message, const PoolItem & pi )
{
  std::string indent("   ");
  message << indent << "--------------------------------------------------" << endl;
  message << indent << (pi->isSystem() ? "i " : "a ") <<  pi->satSolvable().asString() << endl;
  message << indent << pi->summary() << endl;
  if ( pi->isKind<Package>() )
  {
    message << indent << pi->asKind<Package>()->changelog() << endl;
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

  Pathname sysRoot("/");
  bool verbose = false;

  while ( argc && (*argv)[0] == '-' )
  {
    if ( (*argv) == std::string("--root") )
    {
      --argc,++argv;
      if ( ! argc )
	return errexit("--root requires an argument.");

      if ( ! PathInfo( *argv ).isDir() )
	return errexit("--root requires a directory.");

      sysRoot = *argv;
    }
    else if ( (*argv) == std::string("-v") )
    {
      verbose = true;
    }

    --argc,++argv;
  }

  if ( ! argc )
  {
    return usage();
  }

  ///////////////////////////////////////////////////////////////////

  ZConfig::instance();
  sat::Pool satpool( sat::Pool::instance() );

  if ( TestSetup::isTestcase( sysRoot ) )
  {
    message << str::form( "*** Load Testcase from '%s'", sysRoot.c_str() ) << endl;
    TestSetup test;
    test.loadTestcaseRepos( sysRoot );
  }
  else if ( TestSetup::isTestSetup( sysRoot ) )
  {
    message << str::form( "*** Load TestSetup from '%s'", sysRoot.c_str() ) << endl;
    TestSetup test( sysRoot, Arch_x86_64 );
    test.loadRepos();
  }
  else
  {
    // a system
    message << str::form( "*** Load system at '%s'", sysRoot.c_str() ) << endl;
    if ( true )
    {
      message << "*** load target '" << Repository::systemRepoAlias() << "'\t" << endl;
      getZYpp()->initializeTarget( sysRoot );
      getZYpp()->target()->load();
      message << satpool.systemRepo() << endl;
    }

    if ( true )
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

  ///////////////////////////////////////////////////////////////////

  for ( ; argc; --argc,++argv )
  {
    ui::Selectable::Ptr sel( ui::Selectable::get( IdString( *argv ) ) );
    message << dump( sel ) << endl;
    if ( verbose )
    {
      for_( it, sel->installedBegin(), sel->installedEnd() )
	dumpPi( message, *it );
      for_( it, sel->availableBegin(), sel->availableEnd() )
	dumpPi( message, *it );
    }
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}
