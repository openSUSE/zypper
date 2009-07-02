#define INCLUDE_TESTSETUP_WITHOUT_BOOST
#include "zypp/../tests/lib/TestSetup.h"
#undef  INCLUDE_TESTSETUP_WITHOUT_BOOST

#include "zypp/PoolQuery.h"

using namespace zypp;
using std::flush;

static std::string appname( "patch_find_bug" );

int errexit( const std::string & msg_r = std::string(), int exit_r = 101 )
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
  cerr << "Usage: " << appname << "[OPTIONS] bugnumber..." << endl;
  cerr << "  Find patches refering to bugnumber (substring)." << endl;
  cerr << "  --root SYSROOT: Load system located below directory SYSROOT" << endl;
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

  ZConfig::instance();
  Pathname sysRoot("/");

  if ( (*argv) == std::string("--root") )
  {
    --argc,++argv;
    if ( ! argc )
      return errexit("--root requires an argument.");

    if ( ! PathInfo( *argv ).isDir() )
      return errexit("--root requires a directory.");

    sysRoot = *argv;
    --argc,++argv;
  }

  TestSetup::LoadSystemAt( sysRoot );

  for ( ; argc; --argc,++argv )
  {
    PoolQuery q;
    q.setMatchSubstring();
    q.setCaseSensitive( false );
    q.addAttribute( sat::SolvAttr::updateReferenceId, *argv );

    if ( q.empty() )
    {
      cout << "BUG REFERENCE '" << *argv << "': No match found." << endl;
    }
    else
    {
      cout << "BUG REFERENCE '" << *argv << endl;
      for_( it , q.begin(), q.end() )
      {
        // print the solvable that has a match:
        cout << "  - " << *it << endl;

        if ( true )
        {
          // Print details about each match in that solvable:
          for_( d, it.matchesBegin(), it.matchesEnd() )
          {
            // directly access specific attribute like "subFind(updateReferenceType)":
            cout << "    - " << d->inSolvAttr() << "\t\"" << d->asString() << "\" has type \""
                << d->subFind( sat::SolvAttr::updateReferenceType ).asString() << "\"" << endl;

            // list the whole updateReference structure:
            for_( s, d->subBegin(), d->subEnd() )
            {
              cout << "       -" << s.inSolvAttr() << "\t\"" << s.asString() << "\"" << endl;
            }
          }
        }
      }
    }
  }

  ///////////////////////////////////////////////////////////////////
  INT << "===[END]============================================" << endl << endl;
  return 0;
}
