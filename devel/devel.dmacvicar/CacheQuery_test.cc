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
#include "zypp2/cache/QueryFactory.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/data/RecordId.h"
#include "zypp/capability/CapabilityImpl.h"
#include "zypp/base/Measure.h"

using namespace std;
using namespace zypp;
using namespace zypp::debug;
using namespace zypp::capability;


int main(int argc, char **argv)
{
    try
    {
      //ZYpp::Ptr z = getZYpp();

//       CapabilityImpl::Ptr freak = capability::parse( ResTraits<Package>::kind, "libc.so.6");
//       MIL << freak << endl;
//       MIL << "isVer: " << isKind<VersionedCap>(freak) << endl;
//       MIL << "isNam: " << isKind<NamedCap>(freak) << endl;
//       MIL << "isFil: " << isKind<FileCap>(freak) << endl;
//       VersionedCap::Ptr v = asKind<VersionedCap>(freak);
//       DBG << v << endl;

      Pathname dbfile = Pathname(getenv("PWD")) + "data.db";
      cout << 1 << endl;
      zypp::cache::QueryFactory query(getenv("PWD"));
      //zypp::cache::CacheInitializer init( "/", dbfile );
      //t.tick("init sqlite database");
      int i = 1;
      //for ( ; i < 19000; i++ ) {
        zypp::cache::CapabilityQuery capquery = query.createCapabilityQuery( );
        //cout << capquery.value() << endl;
        //cout << 2 << endl;
        while ( capquery.read() )
        {
          //cout << 3 << endl;
          //cout << capquery.value() << endl;
        }
      //}
    }
    catch ( const Exception &e )
    {
      cout << e.msg() << endl;
    }
    catch ( const std::exception &e )
    {
      cout << e.what() << endl;
    }
    
    return 0;
}



