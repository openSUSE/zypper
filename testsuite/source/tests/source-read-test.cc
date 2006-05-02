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

#include "testsuite/utils/TestUtils.h"

using namespace std;
using namespace zypp;

using namespace zypp::testsuite::utils;

bool CustomSort(const std::string &a, const std::string &b)
{
  return a < b; //example comparison.
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
