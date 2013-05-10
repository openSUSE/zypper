#include "Tools.h"

#include <zypp/ResObjects.h>
#include <zypp/sat/WhatObsoletes.h>


static std::string appname( __FILE__ );
static TestSetup test;

///////////////////////////////////////////////////////////////////

#define OUT   USR
#define HEADL SEC << "===> "

inline std::ostream & errmessage( const std::string & msg_r = std::string() )
{
  cerr << "*** ";
  if ( ! msg_r.empty() )
    cerr << msg_r << endl;
  return cerr;
}

int usage( const std::string & msg_r = std::string(), int exit_r = 100 )
{
  if ( ! msg_r.empty() )
  {
    cerr << endl;
    errmessage( msg_r );
    cerr << endl;
  }
  cerr << "Usage: " << appname << " TESTCASE" << endl;
  cerr << "  Load and process testcase." << endl;
  return exit_r;
}

///////////////////////////////////////////////////////////////////

bool upgrade()
{
  bool rres = false;
  {
    zypp::base::LogControl::TmpLineWriter shutUp;
    rres = getZYpp()->resolver()->doUpgrade();
  }
  if ( ! rres )
  {
    ERR << "upgrade " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }
  MIL << "upgrade " << rres << endl;
  return true;
}

bool solve()
{
  static unsigned run = 0;
  USR << "Solve " << run++ << endl;
  bool rres = false;
  {
    zypp::base::LogControl::TmpLineWriter shutUp;
    rres = getZYpp()->resolver()->resolvePool();
  }
  if ( ! rres )
  {
    ERR << "resolve " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////

/**
*/
struct ArgList
{
  typedef std::vector<std::string>::const_iterator const_iterator;

  ArgList()
  {}

  ArgList( const std::string & line_r )
  { str::splitEscaped( line_r, std::back_inserter(_argv) ); }

  const_iterator begin() const { const_iterator ret =_argv.begin(); for ( unsigned i = _carg; i; --i ) ++ret; return ret; }
  const_iterator end()   const { return _argv.end(); }

  void     clear()       { _argv.clear(); _carg = 0; }
  bool     empty() const { return _argv.size() == _carg; }
  unsigned size()  const { return _argv.size() - _carg; }

  std::string &       operator[]( int idx )       { return _argv[_carg+idx]; }
  const std::string & operator[]( int idx ) const { return _argv[_carg+idx]; }

  std::string at( int idx ) const { return _carg+idx < _argv.size() ? _argv[_carg+idx] : std::string(); }

  unsigned carg() const { return _carg; }
  void poparg( int cnt = 1 ) { _carg = arange( _carg + cnt ); }

  public:
    std::vector<std::string> &       get()       { return _argv; }
    const std::vector<std::string> & get() const { return _argv; }
 private:
   unsigned arange( int idx ) const { return idx < 0 ? 0 : std::min( unsigned(idx), _argv.size() ); }
 private:
    DefaultIntegral<unsigned,0> _carg;
    std::vector<std::string> _argv;
};

std::ostream & operator<<( std::ostream & str, const ArgList & obj )
{
  for_( it, 0U, obj.get().size() )
  {
    str << ( it == obj.carg() ? " | " : " " ) << obj.get()[it];
  }
  return str;
}

///////////////////////////////////////////////////////////////////
#define DELGATE(N,F) if ( argv.at(0) == #N ) { argv.poparg(); F( argv ); return; }
///////////////////////////////////////////////////////////////////

void exitCmd( ArgList & argv )
{
  HEADL << argv << endl;
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::TmpLineWriter shutUp;
  ::exit( 0 );
}

///////////////////////////////////////////////////////////////////

void helpCmd( ArgList & argv )
{
  HEADL << argv << endl;
  OUT << "list repos   - list repos in pool" << endl;
  OUT << "list NAME... - list solvables named or providing NAME" << endl;
  OUT << "help         - this" << endl;
  OUT << "exit         - exit" << endl;
}

///////////////////////////////////////////////////////////////////

void listReposCmd( ArgList & argv )
{
  HEADL << "list repos" << endl;

  sat::Pool satpool( test.satpool() );
  for_( it, satpool.reposBegin(), satpool.reposEnd() )
  {
    OUT << *it << endl;
  }
}

void listIdent( IdString ident_r )
{
  HEADL << "list " << ident_r << endl;

  ui::Selectable::Ptr sel( ui::Selectable::get( ident_r ) );
  if ( sel )
  {
    OUT << sel->ident()
        << " I" << sel->installedSize()
        << " A" << sel->availableSize()
        << " " << sel->status()
        << endl;
    for_( it, sel->installedBegin(), sel->installedEnd() )
    {
      OUT << "i " << *it << endl;
    }
    PoolItem cand( sel->candidateObj() );
    for_( it, sel->availableBegin(), sel->availableEnd() )
    {
      OUT << (*it == cand ? "* " : "  ") << *it << endl;
    }
  }

  {
    sat::WhatProvides q( (Capability( ident_r.id() )) );
    bool head = true;
    for_( it, q.begin(), q.end() )
    {
      if ( it->ident() != ident_r )
      {
        if ( head )
        {
          OUT << "provided by:" << endl;
          head = false;
        }
        OUT << "  " << PoolItem( *it ) << endl;
      }
    }
  }
}


void listCmd( ArgList & argv )
{
  DELGATE( repos, listReposCmd );

  for_( it, argv.begin(), argv.end() )
  {
    listIdent( IdString(*it) );
  }
}

///////////////////////////////////////////////////////////////////

void gocmd( ArgList & argv )
{
  if ( argv.empty() )
  {
    helpCmd( argv );
    return;
  }

  switch ( argv[0][0] )
  {
    case 'e':
      DELGATE( exit, exitCmd );
      break;

    case 'h':
      DELGATE( help, helpCmd );
      break;

    case 'l':
      DELGATE( list, listCmd );
      break;
  }
  // no command fall back to list
  listCmd( argv );
}

void goprompt()
{
  std::cin.tie( &std::cout );

  do {
    ArgList argv;
    std::cout << "Hallo : ";
    str::splitEscaped( iostr::getline( std::cin ), std::back_inserter(argv.get()) );
    gocmd( argv );
  } while ( true );

}

///////////////////////////////////////////////////////////////////

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;
  appname = Pathname::basename( argv[0] );
  --argc;
  ++argv;
  ///////////////////////////////////////////////////////////////////

  if ( !argc )
    return usage();

  Pathname mtest( *argv );
  --argc;
  ++argv;

  if ( ! PathInfo( mtest / "solver-test.xml" ).isFile() )
  {
    return usage( "No testcase at " + mtest.asString() );
  }

  ///////////////////////////////////////////////////////////////////

  test.loadTestcaseRepos( mtest ); // <<< repos
#define GOCMD(c) { ArgList argv( #c ); gocmd( argv ); }
  GOCMD( tgt );
  GOCMD( iscsitarget );
  goprompt();

  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::TmpLineWriter shutUp;
  return 0;
}
