
extern "C"
{
#include <sys/file.h>
}
#include <iostream>
#include <fstream>

#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/InterProcessMutex.h"
#include "zypp/base/String.h"

#include "zypp/Pathname.h"
#include "zypp/PathInfo.h"

#define ZYPP_LOCK_FILE "/var/run/zypp.pid"
#define LMIL MIL << "LOCK [" << _name << "] "

using namespace std;

namespace zypp
{
namespace base
{

 ZYppLockedException::ZYppLockedException( const std::string & msg_r,
                                           const std::string &name,
                                           pid_t locker_pid )
    : Exception(msg_r)
    , _locker_pid (locker_pid)
    , _name(name)
{}

ZYppLockedException::~ZYppLockedException() throw()
{}
    

InterProcessMutex::InterProcessMutex( const std::string &name,
                                      int timeout )
  : _clean_lock(false)
  , _zypp_lockfile(0)
  , _locker_pid(0)
  , _name(name)
  , _timeout(timeout)
{

    // get the current pid
    pid_t curr_pid = getpid();
    Pathname lock_file = lockFilePath();
    int totalslept = 0;
    
    while (1)
    {
        if ( PathInfo(lock_file).isExist() )
        {
            LMIL << "found lockfile " << lock_file << std::endl;
            openLockFile("r");
            shLockFile();

            pid_t locker_pid = lockerPid();
            pid_t curr_pid = getpid();
            LMIL << "Locker pid is " << locker_pid << " (our pid: " << curr_pid << ") "<< std::endl;
            _locker_pid = locker_pid;
            // if the same pid has the lock it means we are trying from a new
            // thread and we have to wait.
            // if it is a different one and it is running, then we have to wait
            // too, otherwise we can just take ownership of the lock
            bool locker_running = isProcessRunning(locker_pid);
            LMIL << "locker program is " << (locker_running ? "" : " not") << " running." << endl;
            
            // if the locker process runs and we are not root
            // we can give access without risk.
            if ( locker_running && ( geteuid() != 0 ) )
            {
                LMIL << locker_pid << " is running and has a lock. Access as normal user allowed." << std::endl;
                break;
            }
            // if the locker process is not running we have different cases
            // if we are root, we can clean its lock file, and take ownership of
            // it, which may work or not depending on result of unlink.
            // if the process is not running, and we are not root, we are not able to
            // do much, so we are allowed to continue.
            else if ( ! locker_running )
            {
                if ( geteuid() == 0 )
                {
                    LMIL << locker_pid
                         <<" has a lock, but process is not running. Cleaning lock file." 
                         << std::endl;

                    if ( filesystem::unlink(lock_file) == 0 )
                    {
                        createLockFile();
                        // now open it for reading
                        openLockFile("r");
                        shLockFile();
                        break;
                    }
                    else
                    {
                        ERR << "can't clean lockfile. Sorry, can't create a new lock. Lock [" 
                            << _name << "]  still locked." 
                            << std::endl;
                        ZYPP_THROW(ZYppLockedException(
                                       _("Can't clean lockfile. Sorry, can't create a new lock. Zypp still locked."),
                                       _name, locker_pid));
                    }
                }
                else
                {
                    LMIL << locker_pid 
                         << " not running and has a lock. Access as normal user allowed." 
                         << std::endl;
                    break;
                }                    
            }
            // if the locker process is running and we are root, we have to wait
            // or go away
            else /* if ( locker_running && ( geteuid() == 0 ) ) */
            {
                LMIL << locker_pid << " is running and has a lock." << std::endl;
                
                // abort if we have slept more or equal than the timeout, but
                // not for the case where timeout is negative which means no
                // timeout and therefore we never abort.
                if ( (totalslept >= _timeout) && (_timeout >= 0 ) )
                    ZYPP_THROW(ZYppLockedException(                                       
                                   _("This action is being run by another program already."),
                                   _name, locker_pid));
                        
                // if not, let sleep one second and count it
                LMIL << "waiting 1 second..." << endl;
                unLockFile();
                closeLockFile();
                sleep(1);
                ++totalslept;
                continue;
            }
        }
        else
        {
            MIL << "no lockfile " << lock_file << " found" << std::endl;
            if ( geteuid() == 0 )
            {
                MIL << "running as root. Will attempt to create " << lock_file << std::endl;
                createLockFile();
                // now open it for reading
                openLockFile("r");
                shLockFile();
            }
            else
            {
                MIL << "running as user. Skipping creating " << lock_file << std::endl;
            }
            break;
        }
        break;
    }
    MIL << "Finish constructor" << endl;
    
}

InterProcessMutex::~InterProcessMutex()
{
    try
    {
        pid_t curr_pid = getpid();
        if ( _zypp_lockfile )
        {
            Pathname lock_file = lockFilePath();
            unLockFile();
            closeLockFile();

            if ( _clean_lock )
            {
                MIL << "LOCK [" << _name << "] : cleaning lock file. (" << curr_pid << ")" << std::endl;
                if ( filesystem::unlink(lock_file) == 0 )
                  MIL << "LOCK [" << _name << "] : lockfile cleaned. (" << curr_pid << ")" << std::endl;
                else
                  ERR << "LOCK [" << _name  << "] : cant clean lockfile. (" << curr_pid << ")" << std::endl;
            }
          }
      }
      catch(...) {} // let no exception escape.
}


Pathname InterProcessMutex::lockFilePath() const
{
    return Pathname("/var/run/zypp-" + _name + ".pid");
}    

pid_t
InterProcessMutex::locker_pid() const
{
    return _locker_pid;
}

void InterProcessMutex::openLockFile(const char *mode)
{
    Pathname lock_file = lockFilePath();
    _zypp_lockfile = fopen(lock_file.asString().c_str(), mode);
    if (_zypp_lockfile == 0)
        ZYPP_THROW (Exception( "Cant open " + lock_file.asString() + " in mode " + std::string(mode) ) );
}

void InterProcessMutex::closeLockFile()
{
    fclose(_zypp_lockfile);
}

void InterProcessMutex::shLockFile()
{
    int fd = fileno(_zypp_lockfile);
    int lock_error = flock(fd, LOCK_SH);
    if (lock_error != 0)
        ZYPP_THROW (Exception( "Cant get shared lock"));
    else
        XXX << "locked (shared)" << std::endl;
}

void InterProcessMutex::exLockFile()
{
    int fd = fileno(_zypp_lockfile);
    // lock access to the file
    int lock_error = flock(fd, LOCK_EX);
    if (lock_error != 0)
        ZYPP_THROW (Exception( "Cant get exclusive lock" ));
    else
        XXX << "locked (exclusive)" << std::endl;
}

void InterProcessMutex::unLockFile()
{
    int fd = fileno(_zypp_lockfile);
    // lock access to the file
    int lock_error = flock(fd, LOCK_UN);
    if (lock_error != 0)
        ZYPP_THROW (Exception( "Cant release lock" ));
    else
        XXX << "unlocked" << std::endl;
}

void InterProcessMutex::createLockFile()
{
    pid_t curr_pid = getpid();
    openLockFile("w");
    exLockFile();
    fprintf(_zypp_lockfile, "%ld\n", (long) curr_pid);
    fflush(_zypp_lockfile);
    unLockFile();
    MIL << "written lockfile with pid " << curr_pid << std::endl;
    closeLockFile();
}

bool InterProcessMutex::isProcessRunning(pid_t pid_r)
{
    // it is another program, not me, see if it is still running
    Pathname procdir( Pathname("/proc") / str::numstring(pid_r) );

    PathInfo status( procdir/"status" );
    XXX << "Checking " <<  status << endl;
    bool still_running = status.isExist();

    if ( still_running )
    {
        Pathname p( procdir/"exe" );
        XXX << p << " -> " << filesystem::readlink( p ) << endl;

        p = procdir/"cmdline";
        XXX << p << ": ";
        std::ifstream infile( p.c_str() );
        for( iostr::EachLine in( infile ); in; in.next() )
        {
          XXX << *in << endl;
        }
     }

     return still_running;
}

pid_t InterProcessMutex::lockerPid()
{
    pid_t locker_pid = 0;
    long readpid = 0;

    fscanf(_zypp_lockfile, "%ld", &readpid);
    locker_pid = (pid_t) readpid;
    return locker_pid;
}

bool InterProcessMutex::locked()
{
    return true;
}

}
}


