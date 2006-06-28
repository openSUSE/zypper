#include <getopt.h>

#include <iomanip>
#include <list>
#include <string>

#undef  Y2LOG
#define Y2LOG "PM_installation_sources"
#include <y2util/Y2SLog.h>

#include <y2util/Url.h>

#include <Y2PM.h>
#include <y2pm/PMError.h>
#include <y2pm/InstSrcManager.h>
#include <y2pm/InstSrcDescr.h>

using namespace std;

void usage()
{
  cout << "Usage:" << endl
       << "  installation_sources [-e|-d] -a url   Add source at given URL." << endl
       << "    -e  Enable source. This is the default." << endl
       << "    -d  Disable source." << endl
       << "  installation_sources -s       Show all available sources." << endl;
  exit( 1 );
}

/******************************************************************
**
**
**	FUNCTION NAME : main
**	FUNCTION TYPE : int
**
**	DESCRIPTION :
*/
int main( int argc, char **argv )
{
  MIL << "START" << endl;

  const char *urlStr = 0;

  bool showSources  = false;
  bool addSource    = false;
  bool enableSource = true; // -e per default

  int c;
  while( 1 ) {
    c = getopt( argc, argv, "desa:" );
    if ( c < 0 ) break;

    switch ( c ) {
      case 's':
        showSources = true;
        break;
      case 'a':
        addSource = true;
        urlStr = optarg;
        break;
      case 'd':
        enableSource = false;
        break;
      case 'e':
        enableSource = true;
        break;
      default:
        cerr << "Error parsing command line." << endl;
      case '?':
      case 'h':
        usage();
    }
  }

  if ( showSources && addSource ) usage();
  if ( !showSources && !addSource ) usage();

  Y2PM::noAutoInstSrcManager();
  InstSrcManager &instSrcMgr = Y2PM::instSrcManager();

  InstSrcManager::ISrcIdList sourceIds;

  if ( addSource ) {
    Url url( urlStr );
    if ( !url.isValid() ) {
      cerr << "URL is invalid." << endl;
      exit( 1 );
    }

    PMError error = instSrcMgr.scanMedia( sourceIds, url );
    if ( error ) {
      cerr << error << endl;
      exit( 1 );
    }

    InstSrcManager::ISrcIdList::const_iterator it;
    for( it = sourceIds.begin(); it != sourceIds.end(); ++it ) {
      error = instSrcMgr.enableSource( *it );
      if ( error ) {
        cerr << error << endl;
        exit( 1 );
      }
      instSrcMgr.setAutoenable( *it, enableSource );
    }
    cout << "Added Installation Sources:" << endl;
  }

  if ( showSources ) {
    instSrcMgr.getSources( sourceIds );
    cout << "Installation Sources:" << endl;
  }

  InstSrcManager::ISrcIdList::const_iterator it;
  for( it = sourceIds.begin(); it != sourceIds.end(); ++it ) {
    constInstSrcDescrPtr descr = (*it)->descr();
    cout << ( descr->default_activate() ? "[x]" : "[ ]" );
    cout << ( descr->default_refresh() ? "* " : "  " );
    cout << descr->content_label() << " (" << descr->url() << ")" << endl;
  }

  MIL << "END" << endl;
  return 0;
}
