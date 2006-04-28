// test for SUSE Source
//

#include <string.h>

#include <iostream>
#include <fstream>
#include <map>
#include "zypp/ResObject.h"
#include "zypp/Package.h"
#include "zypp/Selection.h"
#include "zypp/Pattern.h"
#include "zypp/Patch.h"
#include "zypp/Product.h"
#include "zypp/Script.h"
#include "zypp/Message.h"
#include "zypp/Atom.h"
#include "zypp/Dependencies.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/media/MediaManager.h"
#include "zypp/Dependencies.h"

using namespace std;
using namespace zypp;


bool CustomSort(const std::string &a, const std::string &b)
{
  return a < b; //example comparison.
}

static void dumpCapSet( const CapSet &caps, const std::string &deptype)
{
  cout << "   " << "<" << deptype << ">" << std::endl;
  CapSet::iterator it = caps.begin();
  for ( ; it != caps.end(); ++it)
  {
    cout << "    <capability kind=\"" << (*it).kind() << "\" refers=\"" << (*it).refers() << "\">" << (*it).asString() << "</capability>" << std::endl;
  }
  cout << "   " << "</" << deptype << ">" << std::endl;
}

static void dumpDeps( const Dependencies &deps )
{
  dumpCapSet( deps[Dep::PROVIDES], "provides" );
  dumpCapSet( deps[Dep::PREREQUIRES], "prerequires" );
  dumpCapSet( deps[Dep::CONFLICTS], "conflicts" );
  dumpCapSet( deps[Dep::OBSOLETES], "obsoletes" );
  dumpCapSet( deps[Dep::FRESHENS], "freshens" );
  dumpCapSet( deps[Dep::REQUIRES], "requires" );
  dumpCapSet( deps[Dep::RECOMMENDS], "recommends" );
  dumpCapSet( deps[Dep::ENHANCES], "enhances" );
  dumpCapSet( deps[Dep::SUPPLEMENTS], "supplements" );
  dumpCapSet( deps[Dep::SUGGESTS], "suggests" );
}

static void dump( const ResStore &store )
{
  std::list<std::string> resolvables;
  cout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  cout << "<source xmlns=\"http://www.novell.com/zypp/testcases/source\">" << std::endl;
  for (ResStore::const_iterator it = store.begin(); it != store.end(); it++)
  {
    //std::string resolvable_line = "[" + (*it)->kind().asString() + "]" + (*it)->name() + " " + (*it)->edition().asString() + " " + (*it)->arch().asString();
    cout << " <resolvable kind=\"" << (*it)->kind() << "\">" << std::endl;
    cout <<  "  <name>" << (*it)->name() << "</name>" << std::endl;
    cout <<  "  <edition>" << (*it)->edition() << "</edition>" << std::endl;
    cout <<  "  <arch>" << (*it)->arch() << "</arch>" << std::endl;
    cout <<  "  <dependencies>" << std::endl;
    dumpDeps((*it)->deps());
    cout <<  "  </dependencies>" << std::endl;
    cout << "  </resolvable>" << std::endl;
    //std::cout << (**it).deps() << endl;
  }
  cout << "</source>" << std::endl;
  //std::sort( resolvables.begin(), resolvables.end(), CustomSort );
  //for (std::list<std::string>::const_iterator it = resolvables.begin(); it != resolvables.end(); ++it)
  //{
 //   cout << *it << std::endl;
  //}
}


/******************************************************************
**
**
**	FUNCTION NAME : main
**	FUNCTION TYPE : int
**
**	DESCRIPTION :
*/
int main( int argc, char * argv[] )
{
    if (argc < 2) {
	cerr << "Usage: source-read-test [--arch x] <susedir>" << endl;
	exit (1);
    }
    //zypp::base::LogControl::instance().logfile( "-" );

    INT << "===[START]==========================================" << endl;

    ZYpp::Ptr God;
    try {
	God = zypp::getZYpp( );
    }
    catch( const Exception & excpt_r ) {
	ZYPP_CAUGHT( excpt_r );
	cerr << "Can't aquire ZYPP lock" << endl;
	return 1;
    }
    God->initTarget("/", false);
    int argpos = 1;

    if (strcmp( argv[argpos], "--arch") == 0) {
	++argpos;
	God->setArchitecture( Arch( argv[argpos] ) );
	++argpos;
    }
    Pathname p;
    Url url;
    try {
	url = ( argv[argpos++] );
    }
    catch( const Exception & excpt_r ) {
      cerr << "Invalid url " << argv[--argpos] << endl;
      ZYPP_CAUGHT( excpt_r );
      return 1;
    }
    
    string alias("suseparse");
    Locale lang( "en" );

    Pathname cache_dir("");
    Source_Ref src;
    try {
      src = SourceFactory().createFrom(url, p, alias, cache_dir);
    }
    catch( const Exception & excpt_r ) {
      cerr << "Can't access repository" << endl;
      ZYPP_CAUGHT( excpt_r );
      return 1;
    }

    ResStore store = src.resolvables();
    dump(store);
    return 0;
}
