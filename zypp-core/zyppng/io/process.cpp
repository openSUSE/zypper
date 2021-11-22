#include "process.h"
#include <zypp-core/zyppng/io/private/asyncdatasource_p.h>
#include <zypp-core/zyppng/base/EventDispatcher>
#include <zypp-core/zyppng/io/private/abstractspawnengine_p.h>
#include <zypp-core/zyppng/io/AsyncDataSource>
#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>
#include <fcntl.h>

namespace zyppng {

  /*!
   * @todo We will receive a started signal here before exec was called, so sigStarted actually just
   *       signals that the fork has worked out but NOT that the app actually started
   */

  class ProcessPrivate : public AsyncDataSourcePrivate
  {
  public:
    ProcessPrivate( Process &p ) : AsyncDataSourcePrivate(p)
    { }

    void cleanup() {
      _stdinFd  = -1;
      _stderrFd = -1;
      _stdoutFd = -1;
      _pid = -1;
    }

    std::unique_ptr<AbstractSpawnEngine> _spawnEngine = AbstractSpawnEngine::createDefaultEngine();
    zypp::AutoFD _stdinFd  = -1;
    zypp::AutoFD _stderrFd = -1;
    zypp::AutoFD _stdoutFd = -1;
    pid_t _pid = -1;
    Signal<void ()> _sigStarted;
    Signal<void ( int )> _sigFinished;
    Signal<void ()> _sigFailedToStart;

    Process::OutputChannelMode _channelMode = Process::Seperate;
    Process::OutputChannel _currentChannel  = Process::StdOut;
  };

  ZYPP_IMPL_PRIVATE(Process)

  Process::Process() : AsyncDataSource( *( new ProcessPrivate(*this) ) )
  {

  }

  Process::Ptr Process::create()
  {
    return std::shared_ptr<Process>( new Process() );
  }

  Process::~Process()
  {
    Z_D();
    if ( d->_pid >= 0 ) {
      EventDispatcher::instance()->untrackChildProcess( d->_pid );
      DBG << "Process destroyed while still running removing from EventLoop." << std::endl;
    }
  }

  bool Process::start( const char *const *argv )
  {
    Z_D();

    if ( !EventDispatcher::instance() ) {
      ERR << "A valid EventDispatcher needs to be registered before starting a Process" << std::endl;
      return false;
    }

    if ( isRunning() )
      return false;

    // clean up the previous run
    AsyncDataSource::close();
    d->cleanup();

    // create the pipes we need
    auto stdinPipe = Pipe::create( );
    if ( !stdinPipe ) {
      d->_sigFailedToStart.emit();
      return false;
    }

    auto stdoutPipe = Pipe::create( );
    if ( !stdoutPipe ) {
      d->_sigFailedToStart.emit();
      return false;
    }

    int stderr_fd = -1;
    std::optional<Pipe> stderrPipe;
    if ( d->_channelMode == Seperate ) {
      stderrPipe = Pipe::create( );
      if ( !stderrPipe ) {
        d->_sigFailedToStart.emit();
        return false;
      }
      stderr_fd = stderrPipe->writeFd;
    } else {
      stderr_fd = stdoutPipe->writeFd;
    }

    if ( d->_spawnEngine->start( argv, stdinPipe->readFd, stdoutPipe->writeFd, stderr_fd ) ) {

      // if we reach this point the engine guarantees that exec() was successful
      d->_pid = d->_spawnEngine->pid( );

      // register to the eventloop right away
      EventDispatcher::instance()->trackChildProcess( d->_pid, [this]( int, int status ){
        Z_D();
        d->_spawnEngine->setExitStatus( d->_spawnEngine->checkStatus( status ) );
        d->_pid = -1;
        d->_sigFinished.emit( d->_spawnEngine->exitStatus() );
      });

      // make sure the fds we need are kept open
      d->_stdinFd  = std::move( stdinPipe->writeFd );
      d->_stdoutFd = std::move( stdoutPipe->readFd );

      std::vector<int> rFds { d->_stdoutFd };
      if ( stderrPipe ) {
        d->_stderrFd = std::move( stderrPipe->readFd );
        rFds.push_back( d->_stderrFd.value() );
      }

      if ( !openFds( rFds, d->_stdinFd ) ) {
        stop( SIGKILL );
        return false;
      }

      d->_sigStarted.emit();
      return true;
    }
    d->_sigFailedToStart.emit();
    return false;
  }

  void Process::stop( int signal )
  {
    Z_D();
    if ( isRunning() ) {
      ::kill( d->_spawnEngine->pid(), signal );
    }
  }

  bool Process::isRunning()
  {
    Z_D();
    return ( d->_pid > -1 );
  }

  void Process::close ()
  {
    flush();
    stop(SIGKILL);
    d_func()->cleanup();
    AsyncDataSource::close();
  }

  const std::string &Process::executedCommand() const
  {
    return d_func()->_spawnEngine->executedCommand();
  }

  const std::string &Process::execError() const
  {
    return d_func()->_spawnEngine->execError();
  }

  zypp::filesystem::Pathname Process::chroot() const
  {
    return d_func()->_spawnEngine->chroot();
  }

  void Process::setChroot( const zypp::filesystem::Pathname &chroot )
  {
    return d_func()->_spawnEngine->setChroot( chroot );
  }

  bool Process::useDefaultLocale() const
  {
    return d_func()->_spawnEngine->useDefaultLocale();
  }

  void Process::setUseDefaultLocale( bool defaultLocale )
  {
    return d_func()->_spawnEngine->setUseDefaultLocale( defaultLocale );
  }

  Process::Environment Process::environment() const
  {
    return d_func()->_spawnEngine->environment();
  }

  void Process::setEnvironment( const Process::Environment &env )
  {
    return d_func()->_spawnEngine->setEnvironment( env );
  }

  pid_t Process::pid()
  {
    return d_func()->_pid;
  }

  int Process::exitStatus() const
  {
    return d_func()->_spawnEngine->exitStatus();
  }

  bool Process::dieWithParent() const
  {
    return d_func()->_spawnEngine->dieWithParent();
  }

  void Process::setDieWithParent( bool enabled )
  {
    return d_func()->_spawnEngine->setDieWithParent( enabled );
  }

  bool Process::switchPgid() const
  {
    return d_func()->_spawnEngine->switchPgid();
  }

  void Process::setSwitchPgid(bool enabled)
  {
    return d_func()->_spawnEngine->setSwitchPgid( enabled );
  }

  zypp::filesystem::Pathname Process::workingDirectory() const
  {
    return d_func()->_spawnEngine->workingDirectory();
  }

  void Process::setWorkingDirectory(const zypp::filesystem::Pathname &wd)
  {
    return d_func()->_spawnEngine->setWorkingDirectory( wd );
  }

  const std::vector<int> &Process::fdsToMap() const
  {
    return d_func()->_spawnEngine->fdsToMap();
  }

  void Process::addFd(int fd)
  {
    return d_func()->_spawnEngine->addFd( fd );
  }

  int Process::stdinFd()
  {
    return d_func()->_stdinFd;
  }

  int Process::stdoutFd()
  {
    return d_func()->_stdoutFd;
  }

  int Process::stderrFd()
  {
    return d_func()->_stderrFd;
  }

  SignalProxy<void ()> Process::sigStarted()
  {
    return d_func()->_sigStarted;
  }

  SignalProxy<void ()> Process::sigFailedToStart()
  {
    return d_func()->_sigFailedToStart;
  }

  SignalProxy<void (int)> Process::sigFinished()
  {
    return d_func()->_sigFinished;
  }

  Process::OutputChannelMode Process::outputChannelMode() const { return d_func()->_channelMode; }
  void Process::setOutputChannelMode(const OutputChannelMode &outputChannelMode) { d_func()->_channelMode = outputChannelMode; }

}
