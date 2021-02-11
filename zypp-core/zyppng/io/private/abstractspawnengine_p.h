#ifndef ZYPPNG_IO_PRIVATE_ABSTRACTPROCESSBACKEND_H
#define ZYPPNG_IO_PRIVATE_ABSTRACTPROCESSBACKEND_H

#include <string>
#include <vector>
#include <map>

#include <zypp/Pathname.h>

namespace zyppng {

  /*!
   * This implements the basic skeleton of ExternalProgram and Process.
   * Taking care of forking the process and setting up stdout and stderr so both
   * implementations can use the same code
   */
  class AbstractSpawnEngine
  {
  public:

    /**
     * For passing additional environment variables to set
     */
    using Environment = std::map<std::string,std::string>;

    AbstractSpawnEngine();
    virtual ~AbstractSpawnEngine();

    int exitStatus () const;
    void setExitStatus ( const int state );

    const std::string &executedCommand () const;
    const std::string &execError() const;
    void setExecError ( const std::string & str );

    zypp::Pathname chroot() const;
    void setChroot( const zypp::Pathname &chroot );

    bool useDefaultLocale() const;
    void setUseDefaultLocale( bool defaultLocale );

    Environment environment() const;
    void setEnvironment( const Environment &environment );

    pid_t pid   ();

    /** Kickstart the process */
    virtual bool start ( const char *const *argv, int stdin_fd, int stdout_fd, int stderr_fd )  = 0;

    virtual bool isRunning ( bool wait = false ) = 0;

    bool dieWithParent() const;
    void setDieWithParent( bool dieWithParent );

    bool switchPgid() const;
    void setSwitchPgid(bool switchPgid);

    zypp::Pathname workingDirectory() const;
    void setWorkingDirectory(const zypp::Pathname &workingDirectory);

  protected:
    int checkStatus(int status);

  protected:
    bool _useDefaultLocale = false;
    /** Should the process die with the parent process */
    bool _dieWithParent = false;

    bool _switchPgid = false;

    pid_t _pid = -1;
    int _exitStatus = 0;
    /** Remember execution errors like failed fork/exec. */
    std::string _execError;
    /** Store the command we're executing. */
    std::string _executedCommand;
    /** The arguments we want to pass to the program. */
    std::vector<std::string> _args;
    /** Environment variables to set in the new process. */
    Environment _environment;
    /** Path to chroot into */
    zypp::Pathname _chroot;
    /** Working directory */
    zypp::Pathname _workingDirectory;
  };

} // namespace zyppng

#endif // ZYPPNG_IO_PRIVATE_ABSTRACTPROCESSBACKEND_H
