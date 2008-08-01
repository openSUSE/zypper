
#ifndef ZYPP_BASE_INTER_PROCESS_MUTEX_H
#define ZYPP_BASE_INTER_PROCESS_MUTEX_H

#include <string>
#include "zypp/base/Fd.h"
#include "zypp/base/Exception.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/Pathname.h"

namespace zypp
{
namespace base
{

class ZYppLockedException : public Exception
{
public:
    ZYppLockedException( const std::string & msg_r,
                         const std::string &name,
                         pid_t locker_pid );
    virtual ~ZYppLockedException() throw();
    pid_t locker_pid() const { return _locker_pid; }
    std::string name() const { return _name; }
private:
    pid_t _locker_pid;
    std::string _name;
};

/**
 *
 * Inter process scoped lock implementation
 *
 * This mutex will allow only one writer process to
 * reach a critical region protected by a mutex
 * of the same name, if there are no readers
 * at the same time.
 *
 * Multiple readers are allowed if there is no
 * currently a writer.
 *
 */
class InterProcessMutex : private base::NonCopyable
{
public:
   /**
    * Processes can be of two types
    * Reader or Writer
    */
    enum ConsumerType
    {
        Reader,
        Writer
    };

   /**
    * options to alter the mutex behavor
    */
   class Options
   {
   public:
       /**
        * Options for a mutex of type \ref ptype
        * with a given name and timeout.
        * Default is name "zypp" and no timeout
        * (wait till resource is free)
        *
        * The mutex type, Writer or Reader must be
        * given explictly.
        *
        * The mutex will be handled using a lock file
        * located on default library path if the
        * library is running as root, and in users home
        * directory if not.
        *
        */
       Options( ConsumerType ptype,
                const std::string &pname = "zypp",
                int ptimeout = -1 );

       /**
        * set the path where the lockfile is
        * created.
        */
       void setPath( const Pathname &base );

       std::string name;
       int timeout;
       ConsumerType type;
       Pathname base;
   };
    
   /**
    * Creates a mutex with a name and a timeout.
    *
    * default timeout is -1 which means no timeout
    * at all, and the mutex will wait forever if
    * other process is accessing the critical region
    * for a mutex in with the same name.
    *
    * If the timeout is 0, then if the lock is acquired
    * an exception will be thrown inmediately.
    *
    * Otherwise, the timeout exception will come after
    * the timeout is reached.
    *
    */
    InterProcessMutex( const Options &poptions );

    /**
     * Destructor, gives up the lock on the named
     * resource.
     */
    ~InterProcessMutex();

private:
    bool isProcessRunning(pid_t pid_r);
    Pathname lockFilePath() const;
private:
    shared_ptr<Fd> _fd;
    Options _options;
};


} }


#endif

