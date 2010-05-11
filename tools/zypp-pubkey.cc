#define INCLUDE_TESTSETUP_WITHOUT_BOOST
#include "zypp/../tests/lib/TestSetup.h"
#undef  INCLUDE_TESTSETUP_WITHOUT_BOOST
#define message cout
using std::flush;

#include <boost/program_options.hpp>
namespace opt = boost::program_options;

#include <zypp/target/rpm/RpmDb.h>

static std::string appname( "unknown" );

int errexit( const std::string & msg_r = std::string(), int exit_r = 100 )
{
  if ( ! msg_r.empty() )
  {
    cerr << endl << msg_r << endl << endl;
  }
  return exit_r;
}

bool byTTL( const PublicKey & lhs, const PublicKey & rhs )
{
  int cmp = lhs.gpgPubkeyVersion().compare( rhs.gpgPubkeyVersion() );
  if ( cmp ) return cmp < 0;
  return lhs.gpgPubkeyRelease() > rhs.gpgPubkeyRelease(); // intentionally reverse cdate
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  appname = Pathname::basename( argv[0] );
  ///////////////////////////////////////////////////////////////////

  opt::options_description options( "Options" );
  options.add_options()
      ( "key-file",	opt::value<std::vector<std::string> >(),
			"ASCII ascii armored public key file")
      ( "root",		opt::value<std::string>()->default_value( "/" ),
			"Use the rmp database from system rooted at ARG")
      ( "help,?",	"Produce this help message")
      ;

  opt::positional_options_description positional;
  positional.add( "key-file", -1 );

  opt::variables_map vm;
  opt::store( opt::command_line_parser( argc, argv ).options( options ).positional( positional ).run(), vm );
  opt::notify( vm );

  if ( vm.count( "help" ) )
  {
    cerr << "Usage: " << appname << " [OPTIONS] [key-files...]" << endl;
    cerr << "If no key files are given, list info about all gpg-pubkeys in the rpm database." << endl;
    cerr << "Otherwise print info about each key and wheter it is present in the rpm database. " << endl;
    cerr << options << endl;
    return 1;
  }
  ///////////////////////////////////////////////////////////////////

  if ( ! PathInfo( vm["root"].as<std::string>() ).isDir() )
      return errexit("--root requires a directory");

  target::rpm::RpmDb rpmdb;
  rpmdb.initDatabase( vm["root"].as<std::string>() );
  std::list<PublicKey> rpmpubkeys( rpmdb.pubkeys() );
  rpmpubkeys.sort( byTTL );

  if ( ! vm.count( "key-file" ) )
  {
    std::string last;
    for_each_( it, rpmpubkeys )
    {
      if ( last == it->gpgPubkeyVersion() )
	cout << *it << endl;
      else
      {
	cout << dump( *it ) << endl;
	last = it->gpgPubkeyVersion();
      }
    }
    return 0;
  }

  ///////////////////////////////////////////////////////////////////

  const std::vector<std::string> & keyFiles( vm["key-file"].as< std::vector<std::string> >() );
  for_each_( it, vm["key-file"].as< std::vector<std::string> >() )
  {
    cout << "=== " << PathInfo(*it) << endl;
    PublicKey pubkey( *it );
    cout << dump( pubkey ) << endl;

    std::string pubkeyV( pubkey.gpgPubkeyVersion() );
    std::string pubkeyR( pubkey.gpgPubkeyRelease() );
    unsigned count = 0;
    for_each_( rpmpub, rpmpubkeys )
    {
      if ( rpmpub->gpgPubkeyVersion() == pubkeyV )
      {
	int cmp = rpmpub->gpgPubkeyRelease().compare( pubkeyR );
	if ( cmp < 0 )
	  cout << "<<< ";
	else if ( cmp > 0 )
	  cout << ">>> ";
	else
	{
	  ++count;
	  cout << "*** ";
	}
	cout << "gpg-pubkey-" << rpmpub->gpgPubkeyVersion() << "-" << rpmpub->gpgPubkeyRelease() << " " << rpmpub->daysToLive() << endl;
      }
    }
    if ( ! count )
    {
      cout << "*** Not in rpm database." << endl;
    }
    cout << endl;
  }

  ///////////////////////////////////////////////////////////////////
  return 0;
}
