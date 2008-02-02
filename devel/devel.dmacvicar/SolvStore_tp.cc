#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <zypp/base/Measure.h>
#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/Package.h"
#include "zypp/TmpPath.h"
//#include "zypp/cache/CacheInitializer.h"
#include "zypp/cache/SolvStore.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/data/RecordId.h"
#include "zypp/base/Measure.h"
#include "zypp/parser/yum/RepoParser.h"

using namespace std;
using namespace zypp;
using namespace zypp::debug;


int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
      filesystem::TmpDir solvdir;
      zypp::cache::SolvStore store(solvdir.path(), "foo");
      
      Measure cap_insert_timer("Insert Capabilities");
      parser::yum::RepoParser parser("foo", store);
      parser.parse("/mounts/dist/install/stable-x86/suse");
    }
    catch ( const Exception &e )
    {
      cout << e.msg() << endl;
    }
    
    return 0;
}



