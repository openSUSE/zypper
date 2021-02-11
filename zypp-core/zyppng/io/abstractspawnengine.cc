#include "private/abstractspawnengine_p.h"
#include <zypp-core/base/LogControl.h>
#include <zypp-core/base/Gettext.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace zyppng {

  AbstractSpawnEngine::AbstractSpawnEngine()
  {
  }

  AbstractSpawnEngine::~AbstractSpawnEngine()
  { }

  bool AbstractSpawnEngine::switchPgid() const
  {
    return _switchPgid;
  }

  void AbstractSpawnEngine::setSwitchPgid(bool switchPgid)
  {
    _switchPgid = switchPgid;
  }

  zypp::Pathname AbstractSpawnEngine::workingDirectory() const
  {
    return _workingDirectory;
  }

  void AbstractSpawnEngine::setWorkingDirectory(const zypp::Pathname &workingDirectory)
  {
    _workingDirectory = workingDirectory;
  }

  bool AbstractSpawnEngine::dieWithParent() const
  {
    return _dieWithParent;
  }

  void AbstractSpawnEngine::setDieWithParent( bool dieWithParent )
  {
    _dieWithParent = dieWithParent;
  }

  int AbstractSpawnEngine::exitStatus() const
  {
    return _exitStatus;
  }

  void AbstractSpawnEngine::setExitStatus(const int state)
  {
    _exitStatus = state;
  }

  const std::string &AbstractSpawnEngine::executedCommand() const
  {
    return _executedCommand;
  }

  const std::string &AbstractSpawnEngine::execError() const
  {
    return _execError;
  }

  void AbstractSpawnEngine::setExecError(const std::string &str)
  {
    _execError = str;
  }

  zypp::Pathname AbstractSpawnEngine::chroot() const
  {
    return _chroot;
  }

  void AbstractSpawnEngine::setChroot( const zypp::Pathname &chroot )
  {
    _chroot = chroot;
  }

  bool AbstractSpawnEngine::useDefaultLocale() const
  {
    return _useDefaultLocale;
  }

  void AbstractSpawnEngine::setUseDefaultLocale( bool defaultLocale )
  {
    _useDefaultLocale = defaultLocale;
  }

  AbstractSpawnEngine::Environment AbstractSpawnEngine::environment() const
  {
    return _environment;
  }

  void AbstractSpawnEngine::setEnvironment( const Environment &environment )
  {
    _environment = environment;
  }

  pid_t AbstractSpawnEngine::pid()
  {
    if ( !isRunning() )
      return -1;
    return _pid;
  }

  int AbstractSpawnEngine::checkStatus( int status )
  {
    if (WIFEXITED (status))
    {
      status = WEXITSTATUS (status);
      if(status)
      {
          DBG << "Pid " << _pid << " exited with status " << status << std::endl;
          _execError = zypp::str::form( _("Command exited with status %d."), status );
      }
      else
      {
          // if 'launch' is logged, completion should be logged,
          // even if successfull.
          DBG << "Pid " << _pid << " successfully completed" << std::endl;
          _execError.clear(); // empty if running or successfully completed
      }
    }
    else if (WIFSIGNALED (status))
    {
      status = WTERMSIG (status);
      WAR << "Pid " << _pid << " was killed by signal " << status
              << " (" << strsignal(status);
      if (WCOREDUMP (status))
      {
          WAR << ", core dumped";
      }
      WAR << ")" << std::endl;
      _execError = zypp::str::form( _("Command was killed by signal %d (%s)."), status, strsignal(status) );
      status+=128;
    }
    else {
      ERR << "Pid " << _pid << " exited with unknown error" << std::endl;
      _execError = _("Command exited with unknown error.");
    }
    return status;
  }

} // namespace zyppng
