#include "Tools.h"
#include "zypp/pool/GetResolvablesToInsDel.h"

static TestSetup test( Arch_x86_64 );  // use x86_64 as system arch

bool solve()
{
  bool rres = false;
  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    rres = test.resolver().resolvePool();
  }
  if ( ! rres )
  {
    ERR << "resolve " << rres << endl;
    return false;
  }
  MIL << "resolve " << rres << endl;
  return true;
}

int main( int argc, char * argv[] )
try {
  --argc;
  ++argv;
  zypp::base::LogControl::instance().logToStdErr();
  INT << "===[START]==========================================" << endl;

  test.loadTarget(); // initialize and load target
  test.loadRepo( Url("iso:/?iso=/mounts/dist/install/openSUSE-11.1-Beta2-DONTUSE/kiwi.out.dvd-i586.iso") );

  ResPool    pool( test.pool() );
  Resolver & resolver( test.resolver() );

  resolver.addRequire( Capability("glibc") );
  resolver.addRequire( Capability("zlib") );
  resolver.addRequire( Capability("lsb-buildenv") );
  solve();
  vdumpPoolStats( USR << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;

  pool::GetResolvablesToInsDel collect( pool, pool::GetResolvablesToInsDel::ORDER_BY_MEDIANR );
  MIL << "GetResolvablesToInsDel:" << endl << collect << endl;
  if ( 1 )
  {
    // Collect until the 1st package from an unwanted media occurs.
    // Further collection could violate install order.
    bool hitUnwantedMedia = false;
    typedef pool::GetResolvablesToInsDel::PoolItemList PoolItemList;
    PoolItemList::iterator fst=collect._toInstall.end();
    for ( PoolItemList::iterator it = collect._toInstall.begin(); it != collect._toInstall.end(); ++it)
    {
      ResObject::constPtr res( it->resolvable() );

      if ( hitUnwantedMedia
           || ( res->mediaNr() && res->mediaNr() != 1 ) )
      {
        if ( !hitUnwantedMedia )
          fst=it;
        hitUnwantedMedia = true;
      }
      else
      {
      }
    }
    dumpRange( WAR << "toInstall1: " << endl,
               collect._toInstall.begin(), fst ) << endl;
    dumpRange( WAR << "toInstall2: " << endl,
               fst, collect._toInstall.end() ) << endl;
    dumpRange( ERR << "toDelete: " << endl,
               collect._toDelete.begin(), collect._toDelete.end() ) << endl;
  }
  else
  {
    dumpRange( WAR << "toInstall: " << endl,
               collect._toInstall.begin(), collect._toInstall.end() ) << endl;
    dumpRange( ERR << "toDelete: " << endl,
               collect._toDelete.begin(), collect._toDelete.end() ) << endl;
  }

  ///////////////////////////////////////////////////////////////////
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}
catch ( const Exception & exp )
{
  INT << exp << endl << exp.historyAsString();
}
catch (...)
{}

