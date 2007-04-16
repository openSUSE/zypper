
#include <iostream>
#include "zypp/base/Measure.h"
#include "zypp/base/Logger.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"

#include "SUSETagsParser.h"

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
      data::RecordId catalog_id = store.lookupOrAppendCatalog( Url(argv[1]), "/");
      PackagesParser parser( catalog_id, store);
      Measure m;
      parser.start(argv[1]);
      m.elapsed();
    }
    catch ( const Exception &e )
    {
      cout << "ups! " << e.msg() << std::endl;
    }
    return 0;
}

