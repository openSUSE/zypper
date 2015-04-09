#define INCLUDE_TESTSETUP_WITHOUT_BOOST
#include "zypp/../tests/lib/TestSetup.h"
#undef  INCLUDE_TESTSETUP_WITHOUT_BOOST

#include <algorithm>
#include <zypp/PoolQuery.h>
#include <zypp/ResObjects.h>

static std::string appname( "NameReqPrv" );

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
  cerr << "Usage: " << appname << " [--root ROOTDIR] [OPTIONS] NAME... [[OPTIONS] NAME...]..." << endl;
  cerr << "  Load all enabled repositories (no refresh) and search for" << endl;
  cerr << "  occurrences of NAME (regex) in package names or dependencies" << endl;
  cerr << "  --root   Load repos from the system located below ROOTDIR. If ROOTDIR" << endl;
  cerr << "           denotes a sover testcase, the testcase is loaded." << endl;
  cerr << "  --installed Process installed packages only." << endl;
  cerr << "  -i/-I    turn on/off case insensitive search (default on)" << endl;
  cerr << "  -n/-N    turn on/off looking for names       (default on)" << endl;
  cerr << "  -p/-P    turn on/off looking for provides    (default off)" << endl;
  cerr << "  -r/-R    turn on/off looking for requires    (default off)" << endl;
  cerr << "  -c/-C    turn on/off looking for conflicts   (default off)" << endl;
  cerr << "  -o/-O    turn on/off looking for obsoletes   (default off)" << endl;
  cerr << "  -m/-M    turn on/off looking for recommends  (default off)" << endl;
  cerr << "  -s/-S    turn on/off looking for supplements (default off)" << endl;
  cerr << "  -a       short for -n -p -r" << endl;
  cerr << "  -A       short for -n -P -R" << endl;
  cerr << "  -D <pkg> dump dependencies of <pkg>" << endl;
  cerr << "" << endl;
  return exit_r;
}

void tableOut( const std::string & s1 = std::string(),
               const std::string & s2 = std::string(),
               const std::string & s3 = std::string(),
               const std::string & s4 = std::string(),
               const std::string & s5 = std::string() )
{
  message << "  ";
#define TABEL(N) static unsigned w##N = 0; if ( ! s##N.empty() ) w##N = std::max( w##N, unsigned(s##N.size()) ); message << str::form( " %-*s ", w##N, s##N.c_str() )
#define TABER(N) static unsigned w##N = 0; if ( ! s##N.empty() ) w##N = std::max( w##N, unsigned(s##N.size()) ); message << str::form( " %*s ", w##N, s##N.c_str() )
  TABER( 1 ); TABEL( 2 ); TABEL( 3 ); TABEL( 4 ); TABEL( 5 );
#undef TABEL
  message << endl;
}


///////////////////////////////////////////////////////////////////

void dDump( const std::string & spec_r )
{
  message << "DUMP " << spec_r << " {";

  sat::WhatProvides q( Capability::guessPackageSpec( spec_r ) );
  if ( q.empty() )
  {
    message << "}" << endl;
    return;
  }

  for ( const auto & el : q )
  {
    message << endl << "==============================" << endl << dump(el);
    if ( isKind<Pattern>(el) )
    {
      message << endl << "CONTENT: " << make<Pattern>(el)->contents();
    }
  }
  message << endl << "}" << endl;
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
  Pathname sysRoot("/");
  sat::Pool satpool( sat::Pool::instance() );

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
      message << "*** load target '" << Repository::systemRepoAlias() << "'\t" << endl;
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

  ///////////////////////////////////////////////////////////////////

  bool ignorecase	( true );
  bool names		( true );
  bool provides		( false );
  bool requires		( false );
  bool conflicts	( false );
  bool obsoletes	( false );
  bool recommends	( false );
  bool supplements	( false );

  for ( ; argc; --argc,++argv )
  {
    if ( (*argv)[0] == '-' )
    {
      switch ( (*argv)[1] )
      {
        case 'a': names =	true, 	requires = provides =	true;	break;
        case 'A': names =	true, 	requires = provides =	false;	break;
	case 'D':
	  if ( argc > 1 )
	  {
	    --argc,++argv;
	    dDump( *argv );
	  }
	  else
	    return errexit("-D <pkgspec> requires an argument.");
	  break;
        case 'i': ignorecase =	true;	break;
        case 'I': ignorecase =	false;	break;
        case 'n': names =	true;	break;
        case 'N': names =	false;	break;
        case 'r': requires =	true;	break;
        case 'R': requires =	false;	break;
        case 'p': provides =	true;	break;
        case 'P': provides =	false;	break;
        case 'c': conflicts =	true;	break;
        case 'C': conflicts =	false;	break;
        case 'o': obsoletes =	true;	break;
        case 'O': obsoletes =	false;	break;
        case 'm': recommends =	true;	break;
        case 'M': recommends =	false;	break;
        case 's': supplements =	true;	break;
        case 'S': supplements =	false;	break;
      }
      continue;
    }

    PoolQuery q;
    if ( onlyInstalled )
      q.setInstalledOnly();
    std::string qstr( *argv );

    if ( *argv == ResKind::product )
    {
      q.addKind( ResKind::product );
    }
    else if ( *argv == ResKind::patch )
    {
      q.addKind( ResKind::patch );
    }
    else if ( *argv == ResKind::pattern )
    {
      q.addKind( ResKind::pattern );
    }
    else
    {
      sat::Solvable::SplitIdent ident( qstr );
      if ( ident.kind() != ResKind::package )
      {
	q.addKind( ident.kind() );
	q.addString( ident.name().asString() );
      }
      else
	q.addString( qstr );

      q.setMatchRegex();
      q.setCaseSensitive( ! ignorecase );

      if ( names )
	q.addAttribute( sat::SolvAttr::name );
      if ( provides )
	q.addDependency( sat::SolvAttr::provides );
      if ( requires )
	q.addDependency( sat::SolvAttr::requires );
      if ( conflicts )
	q.addDependency( sat::SolvAttr::conflicts );
      if ( obsoletes )
	q.addDependency( sat::SolvAttr::obsoletes );
      if ( recommends )
	q.addDependency( sat::SolvAttr::recommends );
      if ( supplements )
	q.addDependency( sat::SolvAttr::supplements );
    }

    message << *argv << " [" << (ignorecase?'i':'_') << (names?'n':'_') << (requires?'r':'_') << (provides?'p':'_')
    << (conflicts?'c':'_') << (obsoletes?'o':'_') << (recommends?'m':'_') << (supplements?'s':'_') << "] {" << endl;

    for_( it, q.begin(), q.end() )
    {
      tableOut( str::numstring( it->id() ), it->asString(),
		str::form( "(%d)%s", it->repository().info().priority(), it->repository().name().c_str() ),
		str::numstring( PoolItem(*it)->buildtime() ) );
      tableOut( "", "",
		it->vendor().asString() );
      if ( ! it.matchesEmpty() )
      {
	for_( match, it.matchesBegin(), it.matchesEnd() )
	{
	  tableOut( "", "", match->inSolvAttr().asString().substr( 9, 3 )+": " +match->asString() );
	}
      }
    }

    message << "}" << endl;
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}
