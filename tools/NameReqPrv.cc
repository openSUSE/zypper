#define INCLUDE_TESTSETUP_WITHOUT_BOOST
#include "zypp/../tests/lib/TestSetup.h"
#undef  INCLUDE_TESTSETUP_WITHOUT_BOOST

#include <zypp/PoolQuery.h>

static std::string appname( "NameReqPrv" );

void message( const std::string & msg_r )
{
  cerr << "*** " << msg_r << endl;
}

int usage( const std::string & msg_r = std::string(), int exit_r = 100 )
{
  if ( ! msg_r.empty() )
  {
    cerr << endl;
    message( msg_r );
    cerr << endl;
  }
  cerr << "Usage: " << appname << " [OPTIONS] NAME... [[OPTIONS] NAME...]..." << endl;
  cerr << "  Load all enabled repositories (no refresh) and search for" << endl;
  cerr << "  occurrences of NAME (substring) in package names, provides or" << endl;
  cerr << "  requires." << endl;
  cerr << "  -i/-I    turn on/off case insensitive search (default on)" << endl;
  cerr << "  -n/-N    turn on/off looking for names       (default on)" << endl;
  cerr << "  -p/-P    turn on/off looking for provides    (default off)" << endl;
  cerr << "  -r/-R    turn on/off looking for requires    (default off)" << endl;
  cerr << "TODO: Waiting for PoolQuery::allMatches switch and need to beautify output." << endl;
  cerr << "" << endl;
  return exit_r;
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

  ZConfig::instance();
  Pathname sysRoot( "/" );

  if ( 1 )
  {
    message( "*** load target" );
    getZYpp()->initializeTarget( sysRoot );
    getZYpp()->target()->load();
  }

  if ( 1 )
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
        message( str::form( "*** omit uncached repo '%s' (do 'zypper refresh')", nrepo.name().c_str() ) );
        continue;
      }

      message( str::form( "*** load repo '%s'", nrepo.name().c_str() ) );
      try
      {
        repoManager.loadFromCache( nrepo );
      }
      catch ( const Exception & exp )
      {
        message( exp.asString() + "\n" + exp.historyAsString() );
        message( str::form( "*** omit broken repo '%s' (do 'zypper refresh')", nrepo.name().c_str() ) );
        continue;
      }
    }
  }

  sat::Pool satpool( sat::Pool::instance() );
  dumpRange( cerr, satpool.reposBegin(), satpool.reposEnd() ) << endl;
  ///////////////////////////////////////////////////////////////////

  bool ignorecase( true );
  bool names     ( true );
  bool provides  ( false );
  bool requires  ( false );

  for ( ; argc; --argc,++argv )
  {
    if ( (*argv)[0] == '-' )
    {
      switch ( (*argv)[1] )
      {
        case 'i': ignorecase =	true;	break;
        case 'I': ignorecase =	false;	break;
        case 'n': names =	true;	break;
        case 'N': names =	false;	break;
        case 'r': requires =	true;	break;
        case 'R': requires =	false;	break;
        case 'p': provides =	true;	break;
        case 'P': provides =	false;	break;
      }
      continue;
    }

    PoolQuery q;
    q.addString( *argv );
    q.setMatchSubstring();
    q.setCaseSensitive( ! ignorecase );

    if ( names )
      q.addAttribute( sat::SolvAttr::name );
    if ( provides )
      q.addDependency( sat::SolvAttr::provides );
    if ( requires )
      q.addDependency( sat::SolvAttr::requires );

    cerr << *argv << " [" << (ignorecase?'i':'_') << (names?'n':'_') << (requires?'r':'_') << (provides?'p':'_') << "] {" << endl;

    for_( it, q.begin(), q.end() )
    {
      cerr << "  " << it << endl;
    }

    cerr << "}" << endl;
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}