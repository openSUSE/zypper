#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <zypp/base/Measure.h>
#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/detail/PackageImplIf.h"
#include "zypp/Package.h"

#include "zypp2/cache/CacheInitializer.h"
#include "zypp2/cache/CacheStore.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/data/RecordId.h"

using namespace std;
using namespace zypp;
using namespace zypp::debug;


int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
      Pathname dbfile = Pathname(getenv("PWD")) + "data.db";
      zypp::cache::CacheStore store(getenv("PWD"));
      //zypp::cache::CacheInitializer init( "/", dbfile );
      //t.tick("init sqlite database");
      
      std::ifstream file((Pathname(SRC_DIR) + "/names").asString().c_str());
      std::string buffer;
      while (file && !file.eof())
      {
        getline(file, buffer);
        data::RecordId id = store.lookupOrAppendName(buffer);
      }
    }
    catch ( const Exception &e )
    {
      cout << e.msg() << endl;
    }
    
    return 0;
}



