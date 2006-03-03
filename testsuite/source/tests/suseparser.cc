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
    int pchcount = 0;
    int prdcount = 0;
    int scrcount = 0;
    int msgcount = 0;
    int atmcount = 0;
    for (ResStore::iterator it = store.begin(); it != store.end(); ++it) {
	Package::constPtr pkg = asKind<Package>(*it);
	Selection::constPtr sel = asKind<Selection>(*it);
	Pattern::constPtr pat = asKind<Pattern>(*it);
	Patch::constPtr pch = asKind<Patch>(*it);
	Product::constPtr prd = asKind<Product>(*it);
	Script::constPtr scr = asKind<Script>(*it);
	Message::constPtr msg = asKind<Message>(*it);
	Atom::constPtr atm = asKind<Atom>(*it);
	if (pkg != NULL) ++pkgcount;
	if (sel != NULL) ++selcount;
	if (pat != NULL) ++patcount;
	if (pch != NULL) ++pchcount;
	if (prd != NULL) ++prdcount;
	if (scr != NULL) ++scrcount;
	if (msg != NULL) ++msgcount;
	if (atm != NULL) ++atmcount;
    }
    std::cout << "Found " << store.size() << " resolvables" << endl;
    std::cout << "\t" << pkgcount << " packages" << endl;
    std::cout << "\t" << selcount << " selections" << endl;
    std::cout << "\t" << patcount << " patterns" << endl;
    std::cout << "\t" << pchcount << " patches" << endl;
    std::cout << "\t" << scrcount << " scripts" << endl;
    std::cout << "\t" << msgcount << " messages" << endl;
    std::cout << "\t" << atmcount << " atoms" << endl;
    std::cout << "\t" << prdcount << " products" << endl;

    std::cout << "Available locales: ";
    ZYpp::LocaleSet locales = God->getAvailableLocales();
    for (ZYpp::LocaleSet::const_iterator it = locales.begin(); it != locales.end(); ++it) {
	if (it != locales.begin()) std::cout << ", ";
	std::cout << it->code();
    }
    std::cout << endl;

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
