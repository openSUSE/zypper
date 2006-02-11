// test for SUSE Source
//

#include <iostream>
#include <fstream>
#include <map>
#include "zypp/base/Logger.h"
#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/media/MediaManager.h"

using namespace std;
using namespace zypp;

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
	cerr << "Usage: suseparse <susedir> <full>" << endl;
	exit (1);
    }

    INT << "===[START]==========================================" << endl;

    Pathname p;
    Url url(argv[1]);
    string alias("suseparse");
    Locale lang( "en" );

    Pathname cache_dir("");
    Source_Ref src( SourceFactory().createFrom(url, p, alias, cache_dir) );

    ResStore store = src.resolvables();
    INT << "Found " << store.size() << " packages" << endl;
    if (argc > 2) {
	int count = 0;
	for (ResStore::iterator it = store.begin(); it != store.end(); ++it) {
	    MIL << ++count << **it << endl;
	}

    }
    else {
    if (store.size() > 0) {
	ResStore::iterator it = store.begin();
	MIL << "First " << (*it)->name() << ":" << (*it)->summary() << endl << (*it)->description() << endl;
    }
    }
    INT << "===[END]============================================" << endl;
    return 0;
}
