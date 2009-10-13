#include "Tools.h"

#include <iostream>

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/String.h>
#include <zypp/base/SerialNumber.h>
#include <zypp/ExternalProgram.h>
#include <zypp/PathInfo.h>
#include <zypp/TmpPath.h>
#include <zypp/ResPoolProxy.h>
#include <zypp/repo/PackageProvider.h>

static const Pathname sysRoot( "/" );

using namespace std;
using namespace zypp;
using namespace zypp::ui;

bool queryInstalledEditionHelper( const std::string & name_r,
                                  const Edition &     ed_r,
                                  const Arch &        arch_r )
{
  if ( ed_r == Edition::noedition )
    return true;
  if ( name_r == "kernel-default" && ed_r == Edition("2.6.22.5-10") )
    return true;
  if ( name_r == "update-test-affects-package-manager" && ed_r == Edition("1.1-6") )
    return true;

  return false;
}

ManagedFile repoProvidePackage( const PoolItem & pi )
{
  ResPool _pool( getZYpp()->pool() );
  repo::RepoMediaAccess _access;

  // Redirect PackageProvider queries for installed editions
  // (in case of patch/delta rpm processing) to rpmDb.
  repo::PackageProviderPolicy packageProviderPolicy;
  packageProviderPolicy.queryInstalledCB( queryInstalledEditionHelper );

  Package::constPtr p = asKind<Package>( pi.resolvable() );

  // Build a repository list for repos
  // contributing to the pool
  repo::DeltaCandidates deltas;//( repo::makeDeltaCandidates( _pool.knownRepositoriesBegin(), _pool.knownRepositoriesEnd() ) );

  repo::PackageProvider pkgProvider( _access, p, deltas, packageProviderPolicy );

  return pkgProvider.providePackage();
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  --argc;
  ++argv;
  zypp::base::LogControl::instance().logToStdErr();
  INT << "===[START]==========================================" << endl;
  ::unsetenv( "ZYPP_CONF" );
  ZConfig::instance();
  TestSetup::LoadSystemAt( sysRoot );
  ///////////////////////////////////////////////////////////////////
  ResPool   pool( ResPool::instance() );
  sat::Pool satpool( sat::Pool::instance() );
  ///////////////////////////////////////////////////////////////////
  dumpRange( USR, satpool.reposBegin(), satpool.reposEnd() ) << endl;
  USR << "pool: " << pool << endl;

  PoolItem pi( getPi<Package>( "amarok" ) );
  SEC << pi << endl;
  ManagedFile f( repoProvidePackage( pi ) );
  SEC << f << endl;
  //f.resetDispose();
  ExternalProgram("find /tmp/var") >> DBG;
  DBG << endl;

  INT << "===[END]============================================" << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}
