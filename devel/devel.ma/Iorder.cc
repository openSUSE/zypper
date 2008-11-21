#include "Tools.h"
#include <zypp/ResObjects.h>

#include "zypp/pool/GetResolvablesToInsDel.h"

Pathname mroot( "/tmp/Bb" );
TestSetup test( mroot, Arch_ppc64 );

bool upgrade()
{
  bool rres = false;
  {
    zypp::base::LogControl::TmpLineWriter shutUp;
    UpgradeStatistics u;
    rres = getZYpp()->resolver()->doUpgrade( u );
  }
  if ( ! rres )
  {
    ERR << "upgrade " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }
  MIL << "upgrade " << rres << endl;
  return true;
}

bool solve()
{
  static unsigned run = 0;
  USR << "Solve " << run++ << endl;
  bool rres = false;
  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    rres = getZYpp()->resolver()->resolvePool();
  }
  if ( ! rres )
  {
    ERR << "resolve " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }
  return true;
}

bool verify()
{
  bool rres = solve();
  ResPool pool( test.pool() );
  for_( it, make_filter_begin<resfilter::ByTransact>(pool),
        make_filter_end<resfilter::ByTransact>(pool) )
  {
    if ( it->status().transacts() &&
         it->status().isBySolver() )
    {
      WAR << "MISSING " << *it << endl;
    }
  }
  return rres;
}

inline void save()
{
  test.poolProxy().saveState();
}

inline void restore()
{
  test.poolProxy().restoreState();
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  Pathname mroot( "/tmp/Bb" );
  TestSetup test( mroot, Arch_ppc64 );
  {
    zypp::base::LogControl::TmpLineWriter shutUp;
    test.loadTarget();
    test.loadTestcaseRepos( "/suse/ma/BUGS/439802/bug439802/YaST2/solverTestcase" );
  }

  save();
  {
    zypp::base::LogControl::TmpLineWriter shutUp;
    getPi<Product>( "SUSE_SLES", Edition("11"), Arch("ppc64") ).status().setTransact( true, ResStatus::USER );
    getPi<Package>( "sles-release", Edition("11-54.3"), Arch("ppc64") ).status().setTransact( true, ResStatus::USER );
    //upgrade();
  }
  ResPool pool( test.pool() );
  vdumpPoolStats( USR << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;

  getZYpp()->resolver()->setIgnoreAlreadyRecommended( true );
  getZYpp()->resolver()->setOnlyRequires( true );

  restore();
  getPi<Package>( "gvfs-backends", Edition("1.0.2-1.4"), Arch("ppc64") ).status().setTransact( true, ResStatus::USER );

  INT << sat::Solvable(909).requires() << endl;

  vdumpPoolStats( SEC << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;
  verify();
  vdumpPoolStats( SEC << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::TmpLineWriter shutUp1;
  return 0;

  pool::GetResolvablesToInsDel collect( pool, pool::GetResolvablesToInsDel::ORDER_BY_MEDIANR );

  {
    for_( it, collect._toDelete.begin(), collect._toDelete.end() )
    {
      restore();
      it->status().setTransact( true, ResStatus::USER );
      SEC << *it << endl;
      vdumpPoolStats( SEC << "Transacting:"<< endl,
                      make_filter_begin<resfilter::ByTransact>(pool),
                      make_filter_end<resfilter::ByTransact>(pool) ) << endl;
      save();
      verify();
    }
  }

  {
    for_( it, collect._toInstall.begin(), collect._toInstall.end() )
    {
      restore();
      it->status().setTransact( true, ResStatus::USER );
      SEC << *it << endl;
      vdumpPoolStats( SEC << "Transacting:"<< endl,
                      make_filter_begin<resfilter::ByTransact>(pool),
                      make_filter_end<resfilter::ByTransact>(pool) ) << endl;
      save();
      verify();
    }
  }


#if 0
  //getPi<>( "", "", Edition(""), Arch("") );
  getPi<Product>( "SUSE_SLES", Edition("11"), Arch("ppc64") ).status().setTransact( true, ResStatus::USER );
  getPi<Package>( "sles-release", Edition("11-54.3"), Arch("ppc64") ).status().setTransact( true, ResStatus::USER );

  ResPool pool( test.pool() );
  vdumpPoolStats( USR << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;
  upgrade();
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
  INT << "===[END]============================================" << endl << endl;
  return 0;
#endif





  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::TmpLineWriter shutUp;
  return 0;
}

