#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

#include <boost/iostreams/device/file_descriptor.hpp>

#include <zypp/base/Logger.h>
#include <zypp/Locale.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/TranslatedText.h>
///////////////////////////////////////////////////////////////////

#include <zypp/base/Logger.h>


#include <map>
#include <set>

#include "zypp/CapFactory.h"
#include "zypp/KeyRing.h"
#include "zypp/Product.h"
#include "zypp/Selection.h"
#include "zypp/Package.h"
#include "zypp/PublicKey.h"

#include "zypp/ZYppFactory.h"

#include "zypp/MediaSetAccess.h"
#include "zypp/SourceFactory.h"
#include "zypp2/source/yum/YUMSourceCacher.h"

#include "testsuite/src/utils/TestUtils.h"

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace zypp::source;
//using namespace DbXml;

#define TestKind Selection

int main()
{
  try
  {
    ZYpp::Ptr z = getZYpp();
    //z->initializeTarget("/");
    
    //SourceManager_Ptr manager = SourceManager::sourceManager();
    
    Source_Ref source = SourceFactory().createFrom( Url("dir:/space/rpms/duncan/vim/i386"), "/", "bleh", Pathname() );
    ResStore store = source.resolvables();
    //zypp::testsuite::utils::dump(store, true, true);
    
    for (ResStore::const_iterator it = store.begin(); it != store.end(); ++it)
    {
      zypp::Package::Ptr res = asKind<zypp::Package>( *it );
      MIL << res->name() << " " << res->edition() << " " << res->location() << std::endl;
    }
    
    /*for (ResStore::resfilter_const_iterator it = z->target()->byKindBegin(ResTraits<TestKind>::kind); it != z->target()->byKindEnd(ResTraits<TestKind>::kind); ++it)
    {
      zypp::TestKind::constPtr res = asKind<const zypp::TestKind>( *it );
      MIL << res->name() << " " << res->edition() << std::endl;
    }*/
    
  }
  catch ( const Exception &e )
  {
    MIL << "Sorry, bye" << endl;
  }
}



