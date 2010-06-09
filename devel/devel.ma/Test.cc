#include "Tools.h"
#include <zypp/ResObjects.h>

#include <zypp/sat/LookupAttr.h>
#include <zypp/PoolQuery.h>
#include <zypp/sat/AttrMatcher.h>

static const Pathname sysRoot( "/tmp/ToolScanRepos" );

void addInstall( const std::string & pkgspec_r )
{
  bool rewrote( false );
  Capability pkgspec( Capability::guessPackageSpec( pkgspec_r, rewrote ) );
  MIL << "Add '" << pkgspec << "' for '" << pkgspec_r << "'" << endl;
  ResPool::instance().resolver().addRequire( pkgspec );
}

void addConflict( const std::string & pkgspec_r )
{
  bool rewrote( false );
  Capability pkgspec( Capability::guessPackageSpec( pkgspec_r, rewrote ) );
  MIL << "Con '" << pkgspec << "' for '" << pkgspec_r << "'" << endl;
  ResPool::instance().resolver().addConflict( pkgspec );
}

bool solve()
{
  bool rres = false;
  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    //ResPool::instance().resolver().setOnlyRequires( true );
    rres = ResPool::instance().resolver().resolvePool();
  }
  if ( ! rres )
  {
    ERR << "resolve " << rres << endl;
    ResPool::instance().resolver().problems();
    return false;
  }
  MIL << "resolve " << rres << endl;
  vdumpPoolStats( USR << "Transacting:"<< endl,
		  make_filter_begin<resfilter::ByTransact>(ResPool::instance()),
                  make_filter_end<resfilter::ByTransact>(ResPool::instance()) ) << endl;

  return true;
}

bool install()
{
  ZYppCommitPolicy pol;
  pol.dryRun( true );
  pol.rpmInstFlags( pol.rpmInstFlags().setFlag( target::rpm::RPMINST_JUSTDB ) );
  SEC << getZYpp()->commit( pol ) << endl;
  return true;
}


/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;
  ///////////////////////////////////////////////////////////////////
  if ( sysRoot == "/" )
    ::unsetenv( "ZYPP_CONF" );
  TestSetup::LoadSystemAt( sysRoot, Arch_x86_64 );
  ///////////////////////////////////////////////////////////////////
  ResPool   pool( ResPool::instance() );
  sat::Pool satpool( sat::Pool::instance() );
  ///////////////////////////////////////////////////////////////////

//   addConflict( "kernel-default" );
//   addConflict( "kernel-default-base" );
  addInstall( "test");
  solve();

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

