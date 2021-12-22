#include "private/abstractspawnengine_p.h"
#include <zypp-core/base/LogControl.h>
#include <zypp-core/base/Gettext.h>
#include <zypp-core/base/String.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "private/forkspawnengine_p.h"

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::exec"

namespace zyppng {


#if ZYPP_HAS_GLIBSPAWNENGINE
  namespace  {

    enum class SpawnEngine {
      GSPAWN,
      PFORK
    };

    SpawnEngine initEngineFromEnv () {
      const std::string fBackend ( zypp::str::asString( ::getenv("ZYPP_FORK_BACKEND") ) );
      if ( fBackend.empty() || fBackend == "auto" || fBackend == "pfork" ) {
        DBG << "Starting processes via posix fork" << std::endl;
        return SpawnEngine::PFORK;
      } else if ( fBackend == "gspawn" ) {
        DBG << "Starting processes via glib spawn" << std::endl;
        return SpawnEngine::GSPAWN;
      }

      DBG << "Falling back to starting process via posix fork" << std::endl;
      return SpawnEngine::PFORK;
    }

    std::unique_ptr<zyppng::AbstractSpawnEngine> engineFromEnv () {
      static const SpawnEngine eng = initEngineFromEnv();
      switch ( eng ) {
        case SpawnEngine::GSPAWN:
          return std::make_unique<zyppng::GlibSpawnEngine>();
        case SpawnEngine::PFORK:
        default:
          return std::make_unique<zyppng::ForkSpawnEngine>();
      }
    }
  }
#else

  std::unique_ptr<zyppng::AbstractSpawnEngine> engineFromEnv () {
    return std::make_unique<zyppng::ForkSpawnEngine>();
  }

#endif

  AbstractSpawnEngine::AbstractSpawnEngine()
  {
  }

  AbstractSpawnEngine::~AbstractSpawnEngine()
  { }

  std::unique_ptr<AbstractSpawnEngine> AbstractSpawnEngine::createDefaultEngine()
  {
    return engineFromEnv();
  }

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

  const std::vector<int> &AbstractSpawnEngine::fdsToMap() const
  {
    return _mapFds;
  }

  void AbstractSpawnEngine::addFd(int fd)
  {
    _mapFds.push_back( fd );
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

  pid_t AbstractSpawnEngine::pid( )
  {
    return _pid;
  }

  int AbstractSpawnEngine::checkStatus( int status )
  {
    if (WIFEXITED (status))
    {
      status = WEXITSTATUS (status);
      if(status)
      {
          WAR << "Pid " << _pid << " exited with status " << status << std::endl;
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
      std::string sigdetail { strsignal(status) };
      if ( WCOREDUMP(status) ) {
        sigdetail += "; Core dumped";
      }
      if ( status == SIGKILL ) {
        sigdetail += "; Out of memory?";
      }
      WAR << "Pid " << _pid << " was killed by signal " << status << " (" << sigdetail << ")" << std::endl;
      _execError = zypp::str::form( _("Command was killed by signal %d (%s)."), status, sigdetail.c_str() );
      status+=128;
    }
    else {
      ERR << "Pid " << _pid << " exited with unknown error" << std::endl;
      _execError = _("Command exited with unknown error.");
    }
    return status;
  }

} // namespace zyppng
