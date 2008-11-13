#include "Tools.h"

#include <zypp/Fetcher.h>

static std::string appname( "ToolProvideSignedDir" );

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
  cerr << "Usage: " << appname << "[OPTIONS] URL..." << endl;
  cerr << "  Load the digested directory at URL to test system below /tmp/" << appname << "." << endl;
  cerr << "  -r ROOT  Use /tmp/ROOT as location of test system (default: " << appname << ")." << endl;
  cerr << "  -c       Clear an existing test system (default)." << endl;
  cerr << "  -n       Do not clear an existing test system but reuse it." << endl;
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
  --argc;
  ++argv;

  if ( ! argc )
  {
    return usage();
  }

  ///////////////////////////////////////////////////////////////////
  Pathname mtmp( "/tmp" );
  Pathname mroot( mtmp/appname );
  bool     oClearRoot = true;

  std::vector<std::string> urls;

  while ( argc )
  {
    if ( argv[0] == std::string("-c") )
    {
      oClearRoot = true;
    }
    else if ( argv[0] == std::string("-n") )
    {
      oClearRoot = false;
    }
    else if ( argv[0] == std::string("-r") )
    {
      --argc;
      ++argv;
      if ( ! argc )
        return usage( "Missing arg to -r ROOT", 101 );

      if ( *(argv[0]) ) // empty
        mroot = mtmp/argv[0];
      else
        mroot = mtmp/appname;
    }
   else
    {
      urls.push_back( argv[0] );
    }
    --argc;
    ++argv;
  }

  if ( urls.empty() )
  {
    return usage( "Missing URLs", 102 );
  }

  ///////////////////////////////////////////////////////////////////

  if ( oClearRoot )
  {
    message( "Clear test system at " + mroot.asString() );
    filesystem::recursive_rmdir( mroot );
  }
  else
  {
    message( "Use test system at " + mroot.asString() );
  }
  filesystem::assert_dir( mroot );

  KeyRing::setDefaultAccept( KeyRing::ACCEPT_UNKNOWNKEY|KeyRing::TRUST_KEY_TEMPORARILY );

  int ret = 0;
  for_( it, urls.begin(), urls.end() )
  {
    message( "Setup " + *it );
    try
    {
      Url url( *it );
      Pathname tdir( mroot/url.getHost()/url.getPathName() );
      filesystem::assert_dir( tdir );

      Fetcher f;
      f.setOptions( Fetcher::AutoAddIndexes );
      f.enqueueDigestedDir( Pathname("."), /*recursive*/false );
      MediaSetAccess ma( url );
      f.start( tdir, ma );
    }
    catch ( const Exception & exp )
    {
      message( exp.asString() + "\n" + exp.historyAsString() );
      ++ret;
    }
  }

  INT << "===[END]============================================" << endl << endl;
  return ret;
}
