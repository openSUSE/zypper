#include <iostream>

#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/Package.h"
#include "zypp/detail/PackageImplIf.h"
#include "zypp/source/PackageDelta.h"
#include "zypp/detail/ImplConnect.h"

#include "zypp/SourceFactory.h"
#include "testsuite/src/utils/TestUtils.h"

using namespace std;
using namespace zypp;
using namespace zypp::detail;
using namespace zypp::source;
using namespace zypp::packagedelta;
//using namespace DbXml;

int main()
{
  try
  {
    ZYpp::Ptr z = getZYpp();
    //z->initializeTarget("/");
    
    //SourceManager_Ptr manager = SourceManager::sourceManager();
    
    //Source_Ref source = SourceFactory().createFrom( Url("dir:/space/rpms/duncan/vim/i386"), "/", "bleh", Pathname() );
    //Source_Ref source = SourceFactory().createFrom( Url("http://ftp.gwdg.de/pub/suse/update/10.1"), "/", "bleh", Pathname() );
    Source_Ref source = SourceFactory().createFrom( Url("dir:/mounts/mirror/SuSE/ftp.suse.com/pub/suse/update/10.1"), "/", "bleh", Pathname() );
    ResStore store = source.resolvables();
    //zypp::testsuite::utils::dump(store, true, true);
    
    for (ResStore::const_iterator it = store.begin(); it != store.end(); ++it)
    {
      zypp::Package::Ptr res = asKind<zypp::Package>( *it );
      std::cout << "------------------------------------------" << std::endl;
      std::cout << res->name() << " " << res->edition() << " " << res->location() << std::endl;
      
      ResImplTraits<Package::Impl>::constPtr impl( ImplConnect::resimpl( res ) );
      std::cout << "  delta rpms -----------------------------" << std::endl;
      for ( std::list<DeltaRpm>::const_iterator it = impl->deltaRpms().begin(); it != impl->deltaRpms().end(); ++it )
      {
        std::cout << "    " << *it << std::endl;
      }
      std::cout << "  patch rpms -----------------------------" << std::endl;
      for ( std::list<PatchRpm>::const_iterator it = impl->patchRpms().begin(); it != impl->patchRpms().end(); ++it )
      {
        //std::cout << "    " <<  *it << std::endl;
      }
    }
        
  }
  catch ( const Exception &e )
  {
    std::cout << "Sorry, bye" << endl;
  }
}



