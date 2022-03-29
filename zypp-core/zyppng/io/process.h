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

#include <zypp-core/zyppng/io/AsyncDataSource>
#include <zypp-core/zyppng/base/Signals>
#include <memory>
#include <map>
#include <signal.h>

namespace zyppng {

  class ProcessPrivate;
  class IODevice;

  class Process : public AsyncDataSource
  {
    ZYPP_DECLARE_PRIVATE(Process);
  public:
    /**
     * For passing additional environment variables to set
     */
    using Environment = std::map<std::string,std::string>;
    using Ptr = std::shared_ptr<Process>;
    using WeakPtr = std::weak_ptr<Process>;

    enum OutputChannelMode {
      Seperate,
      Merged
    };

    enum OutputChannel {
      StdOut = 0,
      StdErr = 1
    };

    static Ptr create ();
    ~Process();

    bool start ( const char *const *argv );
    void stop  ( int signal = SIGTERM );
    bool isRunning ();
    void close () override;

    /*!
     * Blocks until the process has exited, during that time readyRead is not
     * emitted for any read channels. Call \ref readAll to get all remaining data
     * that was written by the process
     */
    void waitForExit ();

    /*!
     * Close the stdin fd of the subprocess. This is required for processes
     * that run until their stdin is closed.
     */
    void closeWriteChannel () override;

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

    int stdinFd ();
    int stdoutFd ();
    int stderrFd ();

    SignalProxy<void ()> sigStarted  ();
    SignalProxy<void ()> sigFailedToStart  ();
    SignalProxy<void ( int )> sigFinished ();

    OutputChannelMode outputChannelMode() const;
    void setOutputChannelMode(const OutputChannelMode &outputChannelMode);

  protected:
    Process();
  };
}

#endif // ZYPPNG_IO_PROCESS_H_DEFINED
