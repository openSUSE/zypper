#include "Tools.h"
#include <zypp/ResObjects.h>

#include <zypp/sat/LookupAttr.h>
#include <zypp/PoolQuery.h>
#include <zypp/ZYppCallbacks.h>

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
  { SEC << endl; }
  virtual void reportend()
  { SEC << endl; }

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
  { SEC << endl; }
  virtual void reportend()
  { SEC << endl; }

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
  static unsigned run = 0;
  USR << "Solve " << run++ << endl;
  bool rres = false;
  {
    zypp::base::LogControl::TmpLineWriter shutUp;
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

bool install()
{
  ZYppCommitPolicy pol;
//pol.dryRun(true);
  pol.rpmInstFlags( pol.rpmInstFlags().setFlag( target::rpm::RPMINST_JUSTDB ) );
  SEC << "START commit..." << endl;
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
  IRR _irr;
  RRR _rrr;
  Pathname mroot( "/tmp/ToolScanRepos" );
  TestSetup test( mroot, Arch_i586 );
  test.loadTarget();
  test.loadRepos();

  ResPool pool( test.pool() );
  ui::Selectable::Ptr sel;

  getSel<Package>( "rpm" )->setToInstall();
  vdumpPoolStats( USR << "Selected:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;

  if ( solve() )
  {
    vdumpPoolStats( USR << "Solved:"<< endl,
                    make_filter_begin<resfilter::ByTransact>(pool),
                    make_filter_end<resfilter::ByTransact>(pool) ) << endl;

    install();
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

