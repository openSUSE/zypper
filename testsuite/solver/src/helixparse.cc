// test for HelixSourceImpl
//

#include <iostream>
#include <fstream>
#include <map>
#include "zypp/base/Logger.h"
#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/media/MediaManager.h"

#include "helix/HelixSourceImpl.h"

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
	cerr << "Usage: rcparse <helixname>" << endl;
	exit (1);
    }

    INT << "===[START]==========================================" << endl;

    Pathname p = argv[1];
    Url url("file:/");
    string alias("helixparse");

#if 0	// old SourceManager
    media::MediaManager mmgr;
    media::MediaId mediaid = mmgr.open(Url("file://"));
    Source_Ref::Impl_Ptr impl = new HelixSourceImpl (mediaid, p);
    SourceFactory _f;
#endif
#if 0 // via SourceFactory
    Pathname cache_dir("");
    Source_Ref src( SourceFactory().createFrom(url, p, alias, cache_dir) );
#endif

    // via HelixSourceImpl

    media::MediaManager mmgr;
    media::MediaId mediaid = mmgr.open(url);
    HelixSourceImpl *impl = new HelixSourceImpl ();
    MIL << "Calling factoryCtor()" << endl;
    impl->factoryCtor (mediaid, p, alias);
    MIL << "Calling createFromImpl()" << endl;
    Source_Ref src( SourceFactory().createFrom(impl) );

    ResStore store = src.resolvables();
    for (ResStore::const_iterator it = store.begin();
	it != store.end(); it++)
    {
	ERR << **it << endl;
    }
    ERR << store << endl;
    INT << "===[END]============================================" << endl;
    return 0;
}
