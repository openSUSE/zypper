/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/

#ifndef ZYPPNG_IO_PROCESS_H_DEFINED
#define ZYPPNG_IO_PROCESS_H_DEFINED

#include <zypp-core/zyppng/base/Base>
#include <zypp-core/zyppng/base/Signals>
#include <memory>
#include <map>
#include <signal.h>

namespace zyppng {

  class ProcessPrivate;
  class IODevice;

  class Process : public Base
  {
    ZYPP_DECLARE_PRIVATE(Process);
  public:
    /**
     * For passing additional environment variables to set
     */
    using Environment = std::map<std::string,std::string>;

    using Ptr = std::shared_ptr<Process>;
    using WeakPtr = std::weak_ptr<Process>;

    static Ptr create ();
    ~Process();

    bool start (const char *const *argv);
    void stop  ( int signal = SIGTERM );
    bool isRunning ();

    const std::string &executedCommand () const;
    const std::string &execError() const;

    zypp::Pathname chroot() const;
    void setChroot( const zypp::Pathname &chroot );

    bool useDefaultLocale() const;
    void setUseDefaultLocale( bool defaultLocale );

    Environment environment() const;
    void setEnvironment( const Environment &environment );

    pid_t pid   ();
    int exitStatus () const;

    bool dieWithParent() const;
    void setDieWithParent(bool enabled );

    bool switchPgid() const;
    void setSwitchPgid(bool enabled);

    zypp::Pathname workingDirectory() const;
    void setWorkingDirectory(const zypp::Pathname &workingDirectory);

    const std::vector<int> &fdsToMap () const;
    void addFd ( int fd );

    std::shared_ptr<IODevice> stdinDevice ();
    std::shared_ptr<IODevice> stdoutDevice ();
    std::shared_ptr<IODevice> stderrDevice ();

    int stdinFd ();
    int stdoutFd ();
    int stderrFd ();

    SignalProxy<void ()> sigStarted  ();
    SignalProxy<void ()> sigFailedToStart  ();
    SignalProxy<void ( int )> sigFinished ();

  protected:
    Process();
  };
}

#endif // ZYPPNG_IO_PROCESS_H_DEFINED
