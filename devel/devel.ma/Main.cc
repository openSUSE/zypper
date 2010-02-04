#include "Tools.h"

#include <zypp/PoolQuery.h>
#include <zypp/target/rpm/librpmDb.h>
#include <zypp/parser/ProductFileReader.h>
#include "zypp/pool/GetResolvablesToInsDel.h"
#include "zypp/sat/WhatObsoletes.h"

///////////////////////////////////////////////////////////////////

//static const Pathname sysRoot( getenv("SYSROOT") ? getenv("SYSROOT") : "/Local/ROOT" );
static const Pathname sysRoot( "/" );

///////////////////////////////////////////////////////////////////

bool solve()
{
  bool rres = false;
  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    //getZYpp()->resolver()->setOnlyRequires( true );
    rres = getZYpp()->resolver()->resolvePool();
  }
  if ( ! rres )
  {
    ERR << "resolve " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }
  MIL << "resolve " << rres << endl;
  return true;
}

bool upgrade()
{
  bool rres = false;
  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    Measure x( "Upgrade" );
    rres = getZYpp()->resolver()->doUpgrade();
  }
  if ( ! rres )
  {
    Measure x( "Upgrade Error" );
    ERR << "upgrade " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }
  MIL << "upgrade " << rres << endl;
  return true;
}

namespace zypp
{
  namespace target
  {
    void writeUpgradeTestcase();
  }
}

int main( int argc, char * argv[] )
try {
  --argc;
  ++argv;
  zypp::base::LogControl::instance().logToStdErr();
  INT << "===[START]==========================================" << endl;
  ///////////////////////////////////////////////////////////////////


  DBG << Pathname("a\\b") << endl;
  DBG << Pathname(".\\a/") << endl;
  DBG << Pathname("/a\\b/c") << endl;

  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;

  ::unsetenv( "ZYPP_CONF" );
  ZConfig::instance();
  ResPool   pool( ResPool::instance() );
  sat::Pool satpool( sat::Pool::instance() );
  ///////////////////////////////////////////////////////////////////
  dumpRange( WAR, satpool.multiversionBegin(), satpool.multiversionEnd() ) << endl;
  TestSetup::LoadSystemAt( sysRoot, Arch_i586 );
  ///////////////////////////////////////////////////////////////////

  dumpRange( USR, satpool.reposBegin(), satpool.reposEnd() ) << endl;
  USR << "pool: " << pool << endl;

  dumpRange( WAR, satpool.multiversionBegin(), satpool.multiversionEnd() ) << endl;

  ui::Selectable::Ptr sel( getSel<Package>( "test" ) );
  WAR << dump( sel ) << endl;

  DBG << sel->setStatus( ui::S_Update, ResStatus::USER ) << endl;
  WAR << dump( sel ) << endl;

  DBG << sel->setStatus( ui::S_Del, ResStatus::USER ) << endl;
  WAR << dump( sel ) << endl;

  DBG << sel->setStatus( ui::S_KeepInstalled, ResStatus::USER ) << endl;
  WAR << dump( sel ) << endl;

  DBG << sel->pickInstall( *(++++sel->availableBegin()) ) << endl;
  WAR << dump( sel ) << endl;

  DBG << sel->pickDelete( *(++sel->installedBegin()) ) << endl;
  WAR << dump( sel ) << endl;

  DBG << sel->pickInstall( *(sel->installedBegin()) ) << endl;
  WAR << dump( sel ) << endl;

  solve();
  WAR << dump( sel ) << endl;
  pool::GetResolvablesToInsDel collect( pool, pool::GetResolvablesToInsDel::ORDER_BY_MEDIANR );

  ///////////////////////////////////////////////////////////////////
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  	return 0;
   {
    PoolQuery q;
    q.addAttribute( sat::SolvAttr::name, "open" );
    q.addKind( ResKind::patch );
    MIL << dump(q) << endl;
  }
  {
    PoolQuery q;
    q.addAttribute( sat::SolvAttr::name, "patch:yast2-samba-server" );
    MIL << dump(q) << endl;
  }

  if ( 0 )
  {
    getZYpp()->resolver()->addRequire( Capability("emacs") );
    solve();
    vdumpPoolStats( USR << "Transacting:"<< endl,
                    make_filter_begin<resfilter::ByTransact>(pool),
                    make_filter_end<resfilter::ByTransact>(pool) ) << endl;
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

