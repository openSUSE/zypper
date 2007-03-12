#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/detail/PackageImplIf.h"
#include "zypp/Package.h"

#include "zypp2/cache/CacheInitializer.h"
#include "zypp/data/ResolvableData.h"

using namespace std;
using namespace zypp;


int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
      Pathname dbfile = Pathname(getenv("PWD")) + "data.db";
      
      zypp::cache::CacheInitializer init( "/", dbfile );
      //t.tick("init sqlite database");
    }
    catch ( const Exception &e )
    {
      cout << e.msg() << endl;
    }
    
    return 0;
}



