#include "Tools.h"

#include <zypp/PoolQuery.h>
#include <zypp/target/rpm/librpmDb.h>

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
  ::unsetenv( "ZYPP_CONF" );
  ZConfig::instance();
  TestSetup::LoadSystemAt( sysRoot, Arch_i586 );
  ///////////////////////////////////////////////////////////////////
  ResPool   pool( ResPool::instance() );
  sat::Pool satpool( sat::Pool::instance() );
  ///////////////////////////////////////////////////////////////////
  dumpRange( USR, satpool.reposBegin(), satpool.reposEnd() ) << endl;
  USR << "pool: " << pool << endl;

  {
    PoolQuery q;
    q.addAttribute( sat::SolvAttr::name, "yast2-samba-server" );
    MIL << dump(q) << endl;
  }
   {
    PoolQuery q;
    q.addAttribute( sat::SolvAttr::name, "yast2-samba-server" );
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

