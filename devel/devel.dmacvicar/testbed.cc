#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/detail/PackageImplIf.h"
#include "zypp/Package.h"

#include "zypp2/cache/SourceCacheInitializer.h"

#include "zypp2/source/sqlite-source/SqliteAccess.h"
#include "zypp2/source/sqlite-source/SqliteSources.h"

#include "zypp/data/ResolvableData.h"

using namespace std;
using namespace zypp;

class TimeCount
{
  public:
  
  TimeCount()
  {
    gettimeofday( &_start, NULL);
  }
  
  void tick( const std::string &msg )
  {
    timeval curr;
    gettimeofday( &curr, NULL);
    timeval tmp = curr;
    MIL << "[TICK] " << msg << " : " << (curr.tv_sec - _start.tv_sec) << "." << curr.tv_usec - _start.tv_usec << endl;
    _start = tmp;
  }
  
  private:
  timeval _start;
};

int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
    
      TimeCount t;
      Pathname dbfile = Pathname(getenv("PWD")) + "data.db";
      SqliteAccess db(dbfile);
      
      if ( ! filesystem::PathInfo(dbfile).isExist() )
      {
        zypp::cache::SourceCacheInitializer init( "/", dbfile );
        t.tick("init sqlite database");
        
        //return 1;
        
        z->initializeTarget("/");
        t.tick("init target");
        ResStore s = z->target()->resolvables();
        t.tick("load target");
        
        SqliteAccess db(dbfile);
        db.openDb(true);
              
        db.insertCatalog( Url("http://www.google.com"), Pathname("/"), "duncan", "A test catalog" );
        db.writeStore( s, ResStatus::uninstalled, "duncan" );
        
        t.tick("write store");
        
    //     for ( ResStore::const_iterator it = s.begin(); it != s.end(); ++it )
    //     {
    //       Package::Ptr p = asKind<Package>(*it);
    //       cout << "ja 1 paquete" << endl;
    //       oa << p;
    //     }
      }
      
      SqliteSources sqsrcs(db.db());
      SourcesList srcs = sqsrcs.sources();
      t.tick("create sources");
      for ( SourcesList::const_iterator it = srcs.begin(); it != srcs.end(); ++it )
      {
        Source_Ref src = *it;
        ResStore dbres = src.resolvables();
        t.tick("read source");
      }
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
    }
    
    return 0;
}



