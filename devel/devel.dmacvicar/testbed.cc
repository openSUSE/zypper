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

using namespace std;
using namespace zypp;
using namespace zypp::source;
using namespace zypp::source::cached;


int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
    
      Pathname dbpath = Pathname(getenv("PWD"));
      
      SourceImpl_Ptr sourceImpl(new CachedSourceImpl(dbpath));
      SourceFactory factory;
      Source_Ref source = factory.createFrom(sourceImpl);
      ResStore dbres = source.resolvables();
      
      MIL << dbres.size() << " resolvables" << endl;

    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      cout << e.msg() << endl;
    }
    
    return 0;
}



