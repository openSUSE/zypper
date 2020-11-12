#define INCLUDE_TESTSETUP_WITHOUT_BOOST
#include "../tests/lib/TestSetup.h"
#undef  INCLUDE_TESTSETUP_WITHOUT_BOOST

#include <algorithm>
#include <zypp/PoolQuery.h>
#include <zypp/ResObjects.h>
#include <zypp/ui/SelectableTraits.h>

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
  cerr << "  occurrences of NAME (regex or -x) in package names or dependencies" << endl;
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
  cerr << "  -e/-E    turn on/off looking for enhan./sugg.(default off)" << endl;
  cerr << "  -a       short for -n -p -r" << endl;
  cerr << "  -A       short for -n -P -R" << endl;
  cerr << "  -x       do exact matching (glob) rather than regex (substring)" << endl;
  cerr << "  -D <pkg> dump dependencies of <pkg>" << endl;
  cerr << "" << endl;
  return exit_r;
}

#define COL_R   "\033[0;31m"
#define COL_G   "\033[0;32m"
#define COL_B   "\033[0;34m"
#define COL_C   "\033[0;36m"
#define COL_M   "\033[0;35m"
#define COL_Y   "\033[0;33m"
#define COL_BL  "\033[0;30m"
#define COL_WH  "\033[1;37m"
#define COL_OFF "\033[0m"

struct PQSort
{
  // std::less semantic
  bool operator()( const sat::Solvable & lhs, const sat::Solvable & rhs ) const
  {
    {
      bool l = lhs.isSystem();
      bool r = rhs.isSystem();
      if ( l != r )
	return r;
    }
    {
      IdString l { lhs.ident() };
      IdString r { rhs.ident() };
      if ( l != r )
	return l < r;
    }
    return avo( PoolItem(lhs), PoolItem(rhs) );
    return lhs.id() > rhs.id();
  }

  ui::SelectableTraits::AVOrder avo;
};

struct Table
{
  using ResultLInes = std::set<std::string>;

  ResultLInes & row( const sat::Solvable & solv_r )
  {
    //smax( _maxSID,  );
    smax( _maxNAME, solv_r.ident().size() + solv_r.edition().size() + solv_r.arch().size() );
    smax( _maxREPO, solv_r.repository().name().size(), solv_r.repository().name() );
    //smax( _maxTIME, );
    //smax( _maxVEND, solv_r.vendor().size() );
    return _map[solv_r];
  }

  void row( PoolQuery::const_iterator it_r )
  {
    ResultLInes & details { row( *it_r ) };
    for_( match, it_r.matchesBegin(), it_r.matchesEnd() ) {
      std::string ent { match->inSolvAttr().asString().substr( 9, 3 )+": " +match->asString() };
      if ( match->inSolvAttr() == sat::SolvAttr::requires
	&& match->inSolvable().prerequires().contains( Capability(match->id()) ) ) {
	static const char * pre = "   " COL_C "[PRE]" COL_OFF;
	ent[3] = '+';
	ent += pre;
      }
      details.insert( std::move(ent) );
    }
  }

  std::ostream & dumpOn( std::ostream & str ) const
  {
    #define S "  "

    #define fmtSID  "%*d"
    #define argSID  _maxSID, slv.id()

    #define fmtNAME COL_B "%s" COL_OFF "-" COL_G "%s" COL_OFF ".%-*s"
    #define argNAME slv.ident().c_str(), slv.edition().c_str(), _maxNAME-slv.ident().size()-slv.edition().size(), slv.arch().c_str()

    #define fmtREPO "(%2d)%-*s"
    #define argREPO slv.repository().info().priority(), _maxREPO, slv.repository().name().c_str()

    #define fmtTIME "%10ld"
    #define argTIME time_t( slv.isSystem() ? slv.installtime() : slv.buildtime() )

    #define fmtVEND "%s"
    #define argVEND slv.vendor().c_str()

    std::string dind( _maxSID + _maxNAME+2/*-.*/ + 2*strlen(S), ' ' );

    for ( const auto & el : _map ) {
      sat::Solvable slv { el.first };
      const char * tagCol = slv.isSystem() ? COL_M : ui::Selectable::get(slv)->identicalInstalled(PoolItem(slv)) ? COL_C : "";

      str << str::form( "%s"    fmtSID S fmtNAME S "%s"    fmtREPO S fmtTIME S fmtVEND COL_OFF "\n",
			tagCol, argSID,  argNAME,  tagCol, argREPO,  argTIME,  argVEND );

      for ( const auto & d : el.second )
	str << dind << d << endl;
    }
    return str;
  }

private:
  static void smax( unsigned & var_r, unsigned val_r, std::string_view n = {} )
  { if ( val_r > var_r ) var_r = val_r; }

private:
  unsigned _maxSID  = 7;	// size + indent
  unsigned _maxNAME = 0;
  unsigned _maxREPO = 0;
  //unsigned _maxTIME = 10;
  //unsigned _maxVEND = 0;
  std::map<sat::Solvable, ResultLInes, PQSort> _map;
};

inline std::ostream & operator<<( std::ostream & str, const Table & table_r )
{ return table_r.dumpOn( str ); }

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
    if ( isKind<Product>(el) )
    {
      message << endl << "REPLACES: " << make<Product>(el)->replacedProducts();
    }
    else if ( isKind<Pattern>(el) )
    {
      message << endl << "CONTENT: " << make<Pattern>(el)->contents();
    }
    else if ( isKind<Patch>(el) )
    {
      message << endl << "STATUS: " << PoolItem(el);
    }
  }
  message << endl << "}" << endl;
}

///////////////////////////////////////////////////////////////////

void dTree( const std::string & cmd_r, const std::string & spec_r )
{
  message << "tree " << spec_r << " {";

  sat::WhatProvides spec( Capability::guessPackageSpec( spec_r ) );
  if ( spec.empty() )
  {
    message << "}" << endl;
    return;
  }

  static const std::list<sat::SolvAttr> attrs {
    sat::SolvAttr::requires,
    sat::SolvAttr::recommends,
    sat::SolvAttr::obsoletes,
    sat::SolvAttr::conflicts,
    sat::SolvAttr::supplements,
  };

  std::map<sat::SolvAttr,std::map<sat::Solvable,std::set<std::string>>> result;	// solvables recommending provided capability
  {
    Table t;
    for ( const auto & el : spec )
    {
      for ( const auto & cap : el.provides() )
      {
	// get attrs matching cap
	for ( const auto & attr : attrs )
	{
	  PoolQuery q;
	  q.addDependency( attr, cap );
	  if ( q.empty() )
	    continue;
	  for_( it, q.begin(), q.end() ) {
	    for_( match, it.matchesBegin(), it.matchesEnd() ) {
	      result[attr][*it].insert( match->inSolvAttr().asString().substr( 9, 3 )+": " +match->asString()
	      + " (" + cap.asString() + ")"
	      );
	    }
	  }
	}
      }
    }
    message << endl << t;
  }

  for ( const auto & attr : attrs )
  {
    if ( result[attr].empty() )
      continue;

    message << endl << "======== " << attr << " by:" << endl;
    Table t;
    for ( const auto & el : result[attr] )
    {
      auto & details { t.row( el.first ) };
      for ( const auto & cap : el.second )
	details.insert( cap );
    }
    message << endl << t << endl;
  }
  message << "}" << endl;
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
    const char * astr = getenv( "ZYPP_TESTSUITE_FAKE_ARCH" );
    if ( !astr || !*astr )
      astr = getenv( "ZYPP_ARCH" );
    if ( !astr || !*astr )
      astr = "x86_64";
    TestSetup test( sysRoot, Arch( astr ) );
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
  bool matechexact	( false );
  bool withSrcPackages	( false );
  bool names		( true );
  bool provides		( false );
  bool requires		( false );
  bool conflicts	( false );
  bool obsoletes	( false );
  bool recommends	( false );
  bool supplements	( false );
  bool enhacements	( false );


  for ( ; argc; --argc,++argv )
  {
    if ( (*argv)[0] == '-' )
    {
      for ( const char * arg = (*argv)+1; *arg != '\0'; ++arg )	// -pr for -p -r
      {
	switch ( *arg )
	{
	  case 'a': names =		true, 	requires = provides =	true;	break;
	  case 'A': names =		true, 	requires = provides =	false;	break;
	  case 'D':
	    if ( argc > 1 )
	    {
	      --argc,++argv;
	      dDump( *argv );
	    }
	    else
	      return errexit("-D <pkgspec> requires an argument.");
	    break;
	  case 'T':
	    if ( argc > 1 )
	    {
	      std::string cmd { *argv };
	      --argc,++argv;
	      dTree( cmd, *argv );
	    }
	    else
	      return errexit("-T <pkgspec> requires an argument.");
	    break;
	  case 'i': ignorecase =	true;	break;
	  case 'I': ignorecase =	false;	break;
	  case 'x': matechexact =	true;	break;
	  case 'n': names =		true;	break;
	  case 'N': names =		false;	break;
	  case 'r': requires =		true;	break;
	  case 'R': requires =		false;	break;
	  case 'p': provides =		true;	break;
	  case 'P': provides =		false;	break;
	  case 'c': conflicts =		true;	break;
	  case 'C': conflicts =		false;	break;
	  case 'o': obsoletes =		true;	break;
	  case 'O': obsoletes =		false;	break;
	  case 'm': recommends =	true;	break;
	  case 'M': recommends =	false;	break;
	  case 's': supplements =	true;	break;
	  case 'S': supplements =	false;	break;
	  case 'e': enhacements =	true;	break;
	  case 'E': enhacements =	false;	break;
	}
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

      if ( matechexact )
	q.setMatchGlob();
      else
	q.setMatchRegex();
      q.setCaseSensitive( ! ignorecase );

      if ( names )
	q.addAttribute( sat::SolvAttr::name );
      if ( provides )
      {
	q.addDependency( sat::SolvAttr::provides );
	q.addDependency( sat::SolvAttr::provides, Capability(qstr) );
      }
      if ( requires )
      {
	q.addDependency( sat::SolvAttr::requires );
	q.addDependency( sat::SolvAttr::requires, Capability(qstr) );
      }
      if ( conflicts )
      {
	q.addDependency( sat::SolvAttr::conflicts );
	q.addDependency( sat::SolvAttr::conflicts, Capability(qstr) );
      }
      if ( obsoletes )
      {
	q.addDependency( sat::SolvAttr::obsoletes );
	q.addDependency( sat::SolvAttr::obsoletes, Capability(qstr) );
      }
      if ( recommends )
      {
	q.addDependency( sat::SolvAttr::recommends );
	q.addDependency( sat::SolvAttr::recommends, Capability(qstr) );
      }
      if ( supplements )
      {
	q.addDependency( sat::SolvAttr::supplements );
	q.addDependency( sat::SolvAttr::supplements, Capability(qstr) );
      }
      if ( enhacements )
      {
	q.addDependency( sat::SolvAttr::enhances );
	q.addDependency( sat::SolvAttr::enhances, Capability(qstr) );
	q.addDependency( sat::SolvAttr::suggests );
	q.addDependency( sat::SolvAttr::suggests, Capability(qstr) );
      }
    }

    message << *argv << " [" << (ignorecase?'i':'_') << (names?'n':'_') << (requires?'r':'_') << (provides?'p':'_')
    << (conflicts?'c':'_') << (obsoletes?'o':'_') << (recommends?'m':'_') << (supplements?'s':'_') << (enhacements?'e':'_')
    << "] {" << endl;

    Table t;
    for_( it, q.begin(), q.end() )
    {
      if ( it->isKind( ResKind::srcpackage ) && !withSrcPackages )
	continue;
      t.row( it );
    }
    message << t << "}" << endl;
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}
