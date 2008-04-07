
#ifndef ZYPP_BASE_INTER_PROCESS_MUTEX_H
#define ZYPP_BASE_INTER_PROCESS_MUTEX_H

#include <string>
#include "zypp/base/Fd.h"
#include "zypp/base/Exception.h"
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
class InterProcessMutex
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
    InterProcessMutex( ConsumerType ctype,
                       const std::string &name = "zypp",
                       int timeout = -1 );

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
    std::string _name;
    int _timeout;
    ConsumerType _type;
};


} }


#endif

