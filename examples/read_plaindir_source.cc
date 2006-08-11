// ZyPP Example
// Read a directory with rpms as a source
// No need for metadata, it is read from the rpms

#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/Package.h"

#include "zypp/SourceFactory.h"
#include "testsuite/src/utils/TestUtils.h"

using namespace std;
using namespace zypp;
using namespace zypp::source;

int
main (int argc, char **argv)
{
  if (argc < 2) {
    cerr << "1|usage: " << argv[0] << " <dir:/somepath>" << endl;
    return 1;
  }
  
  try
  {
    ZYpp::Ptr z = getZYpp();
    
    // plaindir sources are not signed so we don't need to initialize the
    // target to import the system public keys.
    //z->initializeTarget("/");
   
    Source_Ref source = SourceFactory().createFrom( Url(argv[1]), "/", "testsource", Pathname() );
    ResStore store = source.resolvables();
    //zypp::testsuite::utils::dump(store, true, true);
    
    for (ResStore::const_iterator it = store.begin(); it != store.end(); ++it)
    {
      zypp::Package::Ptr res = asKind<zypp::Package>( *it );
      MIL << res->name() << " " << res->edition() << " " << res->location() << std::endl;
    }
        
  }
  catch ( const Exception &e )
  {
    MIL << "Exception ocurred, bye" << endl;
  }
}



