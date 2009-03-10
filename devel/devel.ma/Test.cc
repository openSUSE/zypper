#include "Tools.h"
#include <zypp/ResObjects.h>

#include <zypp/sat/LookupAttr.h>
#include <zypp/PoolQuery.h>

static std::string pidAndAppname()
{
  static std::string _val;
  if ( _val.empty() )
  {
    pid_t mypid = getpid();
    Pathname p( "/proc/"+str::numstring(mypid)+"/exe" );
    Pathname myname( filesystem::readlink( p ) );

    _val += str::numstring(mypid);
    _val += ":";
    _val += myname.basename();
  }
  return _val;
}

bool solve()
{
  static unsigned run = 0;
  USR << "Solve " << run++ << endl;
  bool rres = false;
  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    getZYpp()->resolver()->setOnlyRequires( true );
	getZYpp()->resolver()->setIgnoreAlreadyRecommended( true );
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

typedef sat::ArrayAttr<std::string,std::string> FileList;

#include "zypp/base/IOStream.h"
bool isProcessRunning(pid_t pid_r)
{
  std::string _locker_name;
  // it is another program, not me, see if it is still running
  Pathname procdir( Pathname("/proc")/str::numstring(pid_r) );
  PathInfo status( procdir );
      MIL << "Checking " <<  status << endl;

      if ( ! status.isDir() )
      {
	DBG << "No such process." << endl;
	return false;
      }

      static char buffer[513];
      buffer[0] = buffer[512] = 0;
      // man proc(5): /proc/[pid]/cmdline is empty if zombie.
      if ( std::ifstream( (procdir/"cmdline").c_str() ).read( buffer, 512 ).gcount() > 0 )
      {
	_locker_name = buffer;
	DBG << "Is running: " <<  _locker_name << endl;
	return true;
      }

      DBG << "In zombie state." << endl;
      return false;
    }


/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  INT << isProcessRunning( 26992 ) << endl;
  INT << isProcessRunning( getpid() ) << endl;;
  INT << isProcessRunning( 10430 ) << endl;
  INT << isProcessRunning( 55 ) << endl;
  INT << "===[END]============================================" << endl << endl;
  return 0;


  Pathname mroot( "/tmp/ToolScanRepos" );
  TestSetup test( mroot, Arch_i686, TSO_CLEANROOT );
  test.loadRepo( "/schnell/CD-ARCHIVE/11.1/FTP" );
  test.loadRepo( "/suse/ma/bug-481836_test.solv" );
  //test.loadRepos();

  ResPool pool( test.pool() );
  Resolver & resolver( test.resolver() );

  //dumpRange( USR, pool.begin(), pool.end() );

  resolver.addRequire( Capability("filesystem") );
  resolver.addRequire( Capability("glibc-locale") );
  resolver.addRequire( Capability("glibc.i586 = 2.9-2.8") );
  resolver.addRequire( Capability("xorg-x11-driver-video-openchrome") );
  resolver.addRequire( Capability("zypper") );

  if ( solve() )
  {
    vdumpPoolStats( USR << "Transacting:"<< endl,
                    make_filter_begin<resfilter::ByTransact>(pool),
                    make_filter_end<resfilter::ByTransact>(pool) ) << endl;
  }


  INT << "===[END]============================================" << endl << endl;
  return 0;
}

