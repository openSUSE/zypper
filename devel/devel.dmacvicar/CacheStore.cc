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
      ZYpp::Ptr z = getZYpp();
      Pathname dbfile = Pathname(getenv("PWD")) + "data.db";
      zypp::cache::CacheStore store(getenv("PWD"));
      //zypp::cache::CacheInitializer init( "/", dbfile );
      //t.tick("init sqlite database");

      std::list<CapabilityImpl::Ptr> caps;
      std::ifstream file;
      file.open((Pathname(SRC_DIR) + "/capabilities.txt").asString().c_str());
      std::string buffer;
      Measure cap_parse_timer("Parse capabilities");
      while (file && !file.eof())
      {
        getline(file, buffer);
        CapabilityImpl::Ptr cap = capability::parse( ResTraits<Package>::kind, buffer );
        if ( cap == 0L )
        {
          ZYPP_THROW(Exception("Invalid capability: [" + buffer + "]"));
        }
        caps.push_back(cap);
      }
      cap_parse_timer.elapsed();
      MIL << caps.size() << " capabilities" << endl;
      
      Measure cap_insert_timer("Insert Capabilities");
      for ( list<CapabilityImpl::Ptr>::const_iterator it = caps.begin(); it != caps.end(); ++it )
      {
        CapabilityImpl::Ptr cap = *it;
        if ( ( ! cap ) || ( cap->refers() != ResTraits<Package>::kind ) )
        {
          ERR << "Invalid capability : [" << buffer << "]" << endl;
          continue;
        }
        store.appendDependency( 1, zypp::Dep::REQUIRES, cap );
      }
      cap_insert_timer.elapsed();
      
      return 0;
      
      file.open((Pathname(SRC_DIR) + "/names").asString().c_str());
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



