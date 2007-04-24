
#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/detail/PackageImplIf.h"
#include "zypp/Package.h"
#include "zypp/SourceFactory.h"

#include "zypp2/source/cached/CachedSourceImpl.h"
#include "zypp/data/ResolvableData.h"

#include "RepositoryManager.h"

using namespace std;
using namespace zypp;
using namespace zypp::source;


int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
    
      Pathname dbpath = Pathname(getenv("PWD"));
      
      RepositoryManager manager;
      list<SourceInfo> sources = manager.knownRepositories();
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      cout << e.msg() << endl;
    }
    
    return 0;
}
