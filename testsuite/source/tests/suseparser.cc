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
#include "zypp/Product.h"
#include "zypp/Dependencies.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
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
	cerr << "Usage: suseparse [--arch x] <susedir> [all|reallyall]" << endl;
	exit (1);
    }
    zypp::base::LogControl::instance().logfile( "-" );

    INT << "===[START]==========================================" << endl;

    ZYpp::Ptr God = zypp::getZYpp();

    int argpos = 1;

    if (strcmp( argv[argpos], "--arch") == 0) {
	++argpos;
	God->setArchitecture( Arch( argv[argpos] ) );
	++argpos;
    }
    Pathname p;
    Url url( argv[argpos++] );
    string alias("suseparse");
    Locale lang( "en" );

    Pathname cache_dir("");
    Source_Ref src( SourceFactory().createFrom(url, p, alias, cache_dir) );

    ResStore store = src.resolvables();
    INT << "Found " << store.size() << " resolvables" << endl;
    int pkgcount = 0;
    int selcount = 0;
    int patcount = 0;
    int prdcount = 0;
    for (ResStore::iterator it = store.begin(); it != store.end(); ++it) {
	Package::constPtr pkg = asKind<Package>(*it);
	Selection::constPtr sel = asKind<Selection>(*it);
	Pattern::constPtr pat = asKind<Pattern>(*it);
	Product::constPtr prd = asKind<Product>(*it);
	if (pkg != NULL) ++pkgcount;
	if (sel != NULL) ++selcount;
	if (pat != NULL) ++patcount;
	if (prd != NULL) ++prdcount;
    }
    INT << "Found " << store.size() << " resolvables" << endl;
    INT << "\t" << pkgcount << " packages" << endl;
    INT << "\t" << selcount << " selections" << endl;
    INT << "\t" << patcount << " patterns" << endl;
    INT << "\t" << prdcount << " products" << endl;

    if (argpos < argc) {
	int count = 0;
	for (ResStore::iterator it = store.begin(); it != store.end(); ++it) {
	    ResObject::Ptr robj = *it;
	    MIL << ++count << *robj << endl;
	    if (strcmp(argv[argpos], "reallyall") == 0) {
		const Dependencies & deps = robj->deps();
		MIL << deps << endl;
	    }
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
