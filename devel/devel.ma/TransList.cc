#include "Tools.h"

#include <zypp/PoolQuery.h>
#include <zypp/target/rpm/librpmDb.h>
#include <zypp/parser/ProductFileReader.h>

#include "zypp/sat/WhatObsoletes.h"
#include "zypp/ExternalProgram.h"
#include <zypp/ZYppCallbacks.h>

#include "zypp/sat/Transaction.h"

///////////////////////////////////////////////////////////////////

//static const Pathname sysRoot( getenv("SYSROOT") ? getenv("SYSROOT") : "/Local/ROOT" );
//static const Pathname sysRoot( "/tmp/ToolScanRepos" );
// static const Pathname sysRoot( "/tmp/updateTestcase" );
static const Pathname sysRoot( "/tmp/ToolScanRepos" );

///////////////////////////////////////////////////////////////////
struct IRR : public zypp::callback::ReceiveReport<zypp::target::rpm::InstallResolvableReport>
{
  IRR()
  { connect(); }
#if 0
  enum Action {
    ABORT,  // abort and return error
    RETRY,	// retry
    IGNORE	// ignore the failure
  };

  enum Error {
    NO_ERROR,
    NOT_FOUND, 	// the requested Url was not found
    IO,		// IO error
    INVALID		// th resolvable is invalid
  };

        // the level of RPM pushing
  /** \deprecated We fortunately no longer do 3 attempts. */
  enum RpmLevel {
    RPM,
    RPM_NODEPS,
    RPM_NODEPS_FORCE
  };
#endif

  virtual void reportbegin()
  { /*SEC << endl;*/ }
  virtual void reportend()
  { /*SEC << endl;*/ }

  virtual void start(Resolvable::constPtr /*resolvable*/)
  { INT << endl; }

  virtual bool progress(int /*value*/, Resolvable::constPtr /*resolvable*/)
  {
    static int i = 4;
    if ( --i <= 0 )
    {
      INT << "return abort" << endl;
      return false;
    }
    return true;
  }

  virtual Action problem(Resolvable::constPtr /*resolvable*/, Error /*error*/, const std::string &/*description*/, RpmLevel /*level*/)
  {
    INT << "return abort" << endl;
    return ABORT;
  }

  virtual void finish(Resolvable::constPtr /*resolvable*/, Error /*error*/, const std::string &/*reason*/, RpmLevel /*level*/)
  { INT << endl; }
};

struct RRR : public zypp::callback::ReceiveReport<zypp::target::rpm::RemoveResolvableReport>
{
  RRR()
  { connect(); }
#if 0
  enum Action {
    ABORT,  // abort and return error
    RETRY,	// retry
    IGNORE	// ignore the failure
  };

  enum Error {
    NO_ERROR,
    NOT_FOUND, 	// the requested Url was not found
    IO,		// IO error
    INVALID		// th resolvable is invalid
  };
#endif

  virtual void reportbegin()
  { /*SEC << endl;*/ }
  virtual void reportend()
  { /*SEC << endl;*/ }

  virtual void start( Resolvable::constPtr /*resolvable*/ )
  { INT << endl; }

  virtual bool progress(int /*value*/, Resolvable::constPtr /*resolvable*/)
  { INT << endl; return true; }

  virtual Action problem( Resolvable::constPtr /*resolvable*/ , Error /*error*/ , const std::string &/*description*/ )
  { INT << endl; return ABORT; }

  virtual void finish( Resolvable::constPtr /*resolvable*/ , Error /*error*/ , const std::string &/*reason*/ )
  { INT << endl; }
};


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
    zypp::base::LogControl::TmpLineWriter shutUp;
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

bool install()
{
  ZYppCommitPolicy pol;
  //pol.dryRun( true );
  pol.downloadMode( DownloadAsNeeded );
  pol.rpmInstFlags( pol.rpmInstFlags().setFlag( target::rpm::RPMINST_JUSTDB ) );
  ZYppCommitResult res( getZYpp()->commit( pol ) );
  SEC << res << endl;
  MIL << res.transactionStepList() << endl;
  return true;
}

///////////////////////////////////////////////////////////////////

template <class _Iter>
unsigned count( _Iter begin, _Iter end )
{
  unsigned cnt = 0;
  for_( it, begin, end )
    ++cnt;
  return cnt;
}

///////////////////////////////////////////////////////////////////
int main( int argc, char * argv[] )
try {
  --argc;
  ++argv;
  zypp::base::LogControl::instance().logToStdErr();
  INT << "===[START]==========================================" << endl;
  ///////////////////////////////////////////////////////////////////
  IRR _irr;
  RRR _rrr;
  if ( sysRoot == "/" )
    ::unsetenv( "ZYPP_CONF" );

  sat::Transaction();
  const sat::Transaction a;
  sat::Transaction b;
  sat::Transaction c( a );
  b = a;

  ResPool   pool( ResPool::instance() );
  sat::Pool satpool( sat::Pool::instance() );
  ///////////////////////////////////////////////////////////////////
  dumpRange( WAR << "satpool.multiversion " , satpool.multiversionBegin(), satpool.multiversionEnd() ) << endl;
  TestSetup::LoadSystemAt( sysRoot, Arch_i586 );
  getZYpp()->initializeTarget( sysRoot );

  ///////////////////////////////////////////////////////////////////

  if ( 1 )
  {
    getPi<Product>( "openSUSE-CD-retail" ).status().setToBeInstalled( ResStatus::USER );
    getPi<Pattern>( "devel_qt4" ).status().setToBeInstalled( ResStatus::USER );
//     getPi<Pattern>( "devel_qt4" ).status().setToBeInstalled( ResStatus::USER );
    solve();
    sat::Transaction trans( pool.resolver().getTransaction() );
    trans.order();

    USR << count( trans.actionBegin(), trans.actionEnd() ) << endl;
    USR << count( trans.actionBegin(sat::Transaction::STEP_TODO), trans.actionEnd() ) << endl;
    USR << count( trans.actionBegin(sat::Transaction::STEP_DONE), trans.actionEnd() ) << endl;
    USR << count( trans.actionBegin(sat::Transaction::STEP_ERROR), trans.actionEnd() ) << endl;
    USR << count( trans.actionBegin(sat::Transaction::STEP_TODO|sat::Transaction::STEP_ERROR), trans.actionEnd() ) << endl;
    USR << count( trans.actionBegin(~sat::Transaction::STEP_ERROR), trans.actionEnd() ) << endl;
    USR << count( trans.actionBegin(~sat::Transaction::STEP_TODO), trans.actionEnd() ) << endl;

    //install();
  }

  ///////////////////////////////////////////////////////////////////
  //  ResPoolProxy selpool( pool.proxy() );
  if ( 0 )
  {
    upgrade();
    install();
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

