#include <iostream>
#include <signal.h>
#include <poll.h>
//#include <readline/readline.h>

#include <zypp/base/LogTools.h>
#include <zypp/base/LogControl.h>
#include <zypp/base/Backtrace.h>
#include <zypp/base/Regex.h>

#include "main.h"
#include "Zypper.h"

#include "callbacks/rpm.h"
#include "callbacks/keyring.h"
#include "callbacks/repo.h"
#include "callbacks/media.h"
#include "callbacks/locks.h"
#include "callbacks/job.h"
#include "output/OutNormal.h"
#include "utils/messages.h"

namespace env
{
  std::string SUDO_USER()
  {
     std::string ret;
     if ( const char * env = ::getenv( "SUDO_USER" ) )
       ret = env;
     return ret;
  }
}

void signal_handler( int sig )
{
  Zypper & zypper( Zypper::instance() );
  if ( zypper.runtimeData().waiting_for_input )
  {
    /// \todo try to get rid of this by improving the ^C handling when prompting
    zypper.immediateExit();
  }
  else if ( zypper.exitRequested() > 1 )
  {
    // translators: this will show up if you press ctrl+c twice
    cerr << endl << _("OK OK! Exiting immediately...") << endl;
    WAR << "Immediate Exit requested." << endl;
    zypper.immediateExit();
  }
  else if ( zypper.exitRequested() == 1 )
  {
    // translators: this will show up if you press ctrl+c twice
    cerr << endl << _("Trying to exit gracefully...") << endl;
    WAR << "Trying to exit gracefully..." << endl;
    zypper.requestImmediateExit();
  }
  else
  {
    zypper.requestExit();
  }
}

bool testPipe( int fd_r )
{
  bool ret = true;
  struct pollfd pfd = { fd_r, POLLERR, 0 };
  auto pollRes = 0;

  while ( (pollRes = ::poll( &pfd, 1, 0 )) == -1 && errno == EINTR );

  if ( pollRes >= 0 && pfd.revents & POLLERR )
  {
    WAR << "FD(" << fd_r << ") " << "pipe is broken" << endl;
    ret = false;
  }
  return ret;
}

void signal_nopipe( int sig )
{
  if ( testPipe(STDOUT_FILENO) && testPipe(STDERR_FILENO) )
  {
    // bsc#1145521 - STDOUT/STDERR are OK. Ignore; might be triggered from libcurl.
    DBG << "Ignore SIGPIPE (STDOUT/STDERR are OK)" << endl;
    ::signal( SIGPIPE, signal_nopipe );
  }
  else
  {
    WAR << "Exiting on SIGPIPE..." << endl << dumpBacktrace << endl;
    Zypper & zypper( Zypper::instance() );
    zypper.requestImmediateExit();
  }
}

int main( int argc, char **argv )
{
  struct Bye {
    ~Bye() {
      MIL << "===== Exiting main("<<exitcode<<") =====" << endl;
    }
    int exitcode = -1;
  } say_goodbye;

  // bsc#1183589: Protect against strict/relaxed user umask via sudo
  bool sudo = false;	// will be mentioned in the log
  if ( geteuid() == 0 ) {
    std::string sudouser { env::SUDO_USER() };
    if ( ! sudouser.empty() ) {
      sudo = true;
      mode_t mask = 0022;
      mode_t omask = ::umask( mask );
      if ( mask != omask )
        std::cerr << "zypper: adjusting umask " << str::octstring(omask,3) << " of sudo-user " << sudouser << " to " << str::octstring(mask,3) << " for user root." << endl;
    }
  }

  // set locale
  setlocale( LC_ALL, "" );
  bindtextdomain( PACKAGE, LOCALEDIR );
  textdomain( PACKAGE );

  // logging
  const char *logfile = getenv("ZYPP_LOGFILE");
  if ( logfile == NULL )
    logfile = ZYPPER_LOG;
  base::LogControl::instance().logfile( logfile );

  MIL << "===== Hi, me zypper " VERSION << endl;
  dumpRange( MIL, argv, argv+argc, (sudo ? "===== 'sudo' ": "===== "), "'", "' '", "'", " =====" ) << endl;

  OutNormal out( Out::QUIET );


  if ( ::signal( SIGINT, signal_handler ) == SIG_ERR )
    out.error("Failed to set SIGINT handler.");
  if ( ::signal (SIGTERM, signal_handler ) == SIG_ERR )
    out.error("Failed to set SIGTERM handler.");

  if ( ::signal( SIGPIPE, signal_nopipe ) == SIG_ERR )
    out.error("Failed to set SIGPIPE handler.");

  try
  {
    static RpmCallbacks rpm_callbacks;
    static SourceCallbacks source_callbacks;
    static MediaCallbacks media_callbacks;
    static KeyRingCallbacks keyring_callbacks;
    static DigestCallbacks digest_callbacks;
    static LocksCallbacks locks_callbacks;
    static JobCallbacks job_callbacks;
  }
  catch ( const Exception & e )
  {
    ZYPP_CAUGHT( e );
    out.error( e, "Failed to initialize zypper callbacks." );
    report_a_bug( out );
    return ZYPPER_EXIT_ERR_BUG;
  }
  catch (...)
  {
    out.error( "Failed to initialize zypper callbacks." );
    ERR << "Failed to initialize zypper callbacks." << endl;
    report_a_bug( out );
    return ZYPPER_EXIT_ERR_BUG;
  }

  Zypper & zypper( Zypper::instance() );
  int & exitcode { say_goodbye.exitcode };
  exitcode = zypper.main( argc, argv );
  if ( !exitcode )
    exitcode = zypper.exitInfoCode();	// propagate refresh errors even if main action succeeded
  return exitcode;
}
