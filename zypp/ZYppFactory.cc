/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZYppFactory.cc
 *
*/
extern "C"
{
#include <sys/file.h>
}
#include <iostream>
#include <fstream>
#include <signal.h>

#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/Functional.h"
#include "zypp/base/Backtrace.h"
#include "zypp/PathInfo.h"

#include "zypp/ZYppFactory.h"
#include "zypp/zypp_detail/ZYppImpl.h"
#include "zypp/zypp_detail/ZYppReadOnlyHack.h"

#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>

using boost::interprocess::file_lock;
using boost::interprocess::scoped_lock;
using boost::interprocess::sharable_lock;

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace
  {
    void sigsegvHandler( int sig );
    ::sighandler_t lastSigsegvHandler = ::signal( SIGSEGV, sigsegvHandler );

    /** SIGSEGV handler to log stack trace */
    void sigsegvHandler( int sig )
    {
      INT << "Error: signal " << sig << endl << dumpBacktrace << endl;
      ::signal( SIGSEGV, lastSigsegvHandler );
    }
  }

  namespace env
  {
    /** Hack to circumvent the currently poor --root support. */
    inline Pathname ZYPP_LOCKFILE_ROOT()
    { return getenv("ZYPP_LOCKFILE_ROOT") ? getenv("ZYPP_LOCKFILE_ROOT") : "/"; }
  }

  ///////////////////////////////////////////////////////////////////
  namespace zypp_readonly_hack
  { /////////////////////////////////////////////////////////////////

    static bool active = false;

    void IWantIt()
    {
      active = true;
      MIL << "ZYPP_READONLY promised." <<  endl;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace zypp_readonly_hack
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  /// \class ZYppGlobalLock
  /// \brief Our broken global lock
  ///
  ///////////////////////////////////////////////////////////////////
  class ZYppGlobalLock
  {
  public:
    ZYppGlobalLock()
    : _cleanLock( false )
    , _zyppLockFilePath( env::ZYPP_LOCKFILE_ROOT() / "/var/run/zypp.pid" )
    , _zyppLockFile( NULL )
    , _lockerPid( 0 )
    {
      filesystem::assert_dir(_zyppLockFilePath.dirname() );
    }

    ~ZYppGlobalLock()
    {
	if ( _cleanLock )
	try {
	  // Exception safe access to the lockfile.
	  ScopedGuard closeOnReturn( accessLockFile() );
	  {
	    scoped_lock<file_lock> flock( _zyppLockFileLock );	// aquire write lock
	    // Truncate the file rather than deleting it. Other processes may
	    // still use it to synchronsize.
	    ftruncate( fileno(_zyppLockFile), 0 );
	  }
	  MIL << "Cleanned lock file. (" << getpid() << ")" << std::endl;
	}
	catch(...) {} // let no exception escape.
    }

    pid_t lockerPid() const
    { return _lockerPid; }

    const std::string & lockerName() const
    { return _lockerName; }

    const Pathname & zyppLockFilePath() const
    { return _zyppLockFilePath; }


  private:
    Pathname	_zyppLockFilePath;
    file_lock	_zyppLockFileLock;
    FILE *	_zyppLockFile;

    pid_t	_lockerPid;
    std::string _lockerName;
    bool	_cleanLock;

  private:
    typedef shared_ptr<void> ScopedGuard;

    /** Exception safe access to the lockfile.
     * \code
     *   // Exception safe access to the lockfile.
     *   ScopedGuard closeOnReturn( accessLockFile() );
     * \endcode
     */
    ScopedGuard accessLockFile()
    {
      _openLockFile();
      return ScopedGuard( static_cast<void*>(0),
			  bind( mem_fun_ref( &ZYppGlobalLock::_closeLockFile ), ref(*this) ) );
    }

    /** Use \ref accessLockFile. */
    void _openLockFile()
    {
      if ( _zyppLockFile != NULL )
	return;	// is open

      // open pid file rw so we are sure it exist when creating the flock
      _zyppLockFile = fopen( _zyppLockFilePath.c_str(), "a+" );
      if ( _zyppLockFile == NULL )
	ZYPP_THROW( Exception( "Cant open " + _zyppLockFilePath.asString() ) );
      _zyppLockFileLock = _zyppLockFilePath.c_str();
      MIL << "Open lockfile " << _zyppLockFilePath << endl;
    }

    /** Use \ref accessLockFile. */
    void _closeLockFile()
    {
      if ( _zyppLockFile == NULL )
	return;	// is closed

      clearerr( _zyppLockFile );
      fflush( _zyppLockFile );
      // http://www.boost.org/doc/libs/1_50_0/doc/html/interprocess/synchronization_mechanisms.html
      // If you are using a std::fstream/native file handle to write to the file
      // while using file locks on that file, don't close the file before releasing
      // all the locks of the file.
      _zyppLockFileLock = file_lock();
      fclose( _zyppLockFile );
      _zyppLockFile = NULL;
      MIL << "Close lockfile " << _zyppLockFilePath << endl;
    }


    bool isProcessRunning( pid_t pid_r )
    {
      // it is another program, not me, see if it is still running
      Pathname procdir( "/proc"/str::numstring(pid_r) );
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
	_lockerName = buffer;
	DBG << "Is running: " <<  _lockerName << endl;
	return true;
      }

      DBG << "In zombie state." << endl;
      return false;
    }

    pid_t readLockFile()
    {
      clearerr( _zyppLockFile );
      fseek( _zyppLockFile, 0, SEEK_SET );
      long readpid = 0;
      fscanf( _zyppLockFile, "%ld", &readpid );
      MIL << "read: Lockfile " << _zyppLockFilePath << " has pid " << readpid << " (our pid: " << getpid() << ") "<< std::endl;
      return (pid_t)readpid;
    }

    void writeLockFile()
    {
      clearerr( _zyppLockFile );
      fseek( _zyppLockFile, 0, SEEK_SET );
      ftruncate( fileno(_zyppLockFile), 0 );
      fprintf(_zyppLockFile, "%ld\n", (long)getpid() );
      fflush( _zyppLockFile );
      _cleanLock = true; // cleanup on exit
      MIL << "write: Lockfile " << _zyppLockFilePath << " got pid " <<  getpid() << std::endl;
    }

  public:

    /** Try to aquire a lock.
     * \return \c true if zypp is already locked by another process.
     */
    bool zyppLocked()
    {
      if ( geteuid() != 0 )
	return false;	// no lock as non-root

      // Exception safe access to the lockfile.
      ScopedGuard closeOnReturn( accessLockFile() );
      {
	scoped_lock<file_lock> flock( _zyppLockFileLock );	// aquire write lock

	_lockerPid = readLockFile();
	if ( _lockerPid == 0 )
	{
	  // no or empty lock file
	  writeLockFile();
	  return false;
	}
	else if ( _lockerPid == getpid() )
	{
	  // keep my own lock
	  return false;
	}
	else
	{
	  // a foreign pid in lock
	  if ( isProcessRunning( _lockerPid ) )
	  {
	    WAR << _lockerPid << " is running and has a ZYpp lock. Sorry." << std::endl;
	    return true;
	  }
	  else
	  {
	    MIL << _lockerPid << " is dead. Taking the lock file." << std::endl;
	    writeLockFile();
	    return false;
	  }
	}
      }
      INT << "Oops! We should not be here!" << std::endl;
      return true;
    }

  };

  namespace
  {
    static ZYppGlobalLock & globalLock()
    {
      static ZYppGlobalLock lock;
      return lock;
    }
    bool           _haveZYpp = false;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZYppFactoryException
  //
  ///////////////////////////////////////////////////////////////////

  ZYppFactoryException::ZYppFactoryException( const std::string & msg_r, pid_t lockerPid_r, const std::string & lockerName_r )
    : Exception( msg_r )
    , _lockerPid( lockerPid_r )
    , _lockerName( lockerName_r )
  {}

  ZYppFactoryException::~ZYppFactoryException() throw ()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZYppFactory
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZYppFactory::instance
  //	METHOD TYPE : ZYppFactory
  //
  ZYppFactory ZYppFactory::instance()
  {
    return ZYppFactory();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZYppFactory::ZYppFactory
  //	METHOD TYPE : Ctor
  //
  ZYppFactory::ZYppFactory()
  {

  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZYppFactory::~ZYppFactory
  //	METHOD TYPE : Dtor
  //
  ZYppFactory::~ZYppFactory()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  ZYpp::Ptr ZYppFactory::getZYpp() const
  {
    static ZYpp::Ptr _instance;

    if ( ! _instance )
    {
      if ( geteuid() != 0 )
      {
	MIL << "Running as user. Skip creating " << globalLock().zyppLockFilePath() << std::endl;
      }
      else if ( zypp_readonly_hack::active )
      {
	MIL << "ZYPP_READONLY active." << endl;
      }
      else if ( globalLock().zyppLocked() )
      {
	bool failed = true;
	const long LOCK_TIMEOUT = str::strtonum<long>( getenv( "ZYPP_LOCK_TIMEOUT" ) );
	if ( LOCK_TIMEOUT > 0 )
	{
	  MIL << "Waiting whether pid " << globalLock().lockerPid() << " ends within $LOCK_TIMEOUT=" << LOCK_TIMEOUT << " sec." << endl;
	  unsigned delay = 1;
	  Pathname procdir( "/proc"/str::numstring(globalLock().lockerPid()) );
	  for ( long i = 0; i < LOCK_TIMEOUT; i += delay )
	  {
	    if ( PathInfo( procdir ).isDir() )	// wait for /proc/pid to disapear
	      sleep( delay );
	    else
	    {
	      MIL << "Retry after " << i << " sec." << endl;
	      failed = globalLock().zyppLocked();
	      if ( failed )
	      {
		// another proc locked faster. maybe it ends fast as well....
		MIL << "Waiting whether pid " << globalLock().lockerPid() << " ends within " << (LOCK_TIMEOUT-i) << " sec." << endl;
		procdir = Pathname( "/proc"/str::numstring(globalLock().lockerPid()) );
	      }
	      else
	      {
		MIL << "Finally got the lock!" << endl;
		break;	// gotcha
	      }
	    }
	  }

	}
	if ( failed )
	{
	  std::string t = str::form(_("System management is locked by the application with pid %d (%s).\n"
				      "Close this application before trying again."),
				      globalLock().lockerPid(),
				      globalLock().lockerName().c_str()
				    );
	  ZYPP_THROW(ZYppFactoryException(t, globalLock().lockerPid(), globalLock().lockerName() ));
	}
      }
      // Here we go...
      _instance = new ZYpp( ZYpp::Impl_Ptr(new ZYpp::Impl) );
      if ( _instance )
        _haveZYpp = true;
    }

    return _instance;
  }

  ///////////////////////////////////////////////////////////////////
  //
  bool ZYppFactory::haveZYpp() const
  { return _haveZYpp; }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const ZYppFactory & obj )
  {
    return str << "ZYppFactory";
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
