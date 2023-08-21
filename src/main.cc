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

static const char* exit_requested_once_str;
static const char* exit_requested_twice_str;

void signal_handler( int sig )
{
  // Do NOT write to the logfiles or use a iostream, we can NOT call into malloc here and
  // building a string always does that.
  Zypper & zypper( Zypper::instance( true ) );
  if ( zypper.exitRequested() >= 1 ) {
    write ( STDERR_FILENO, exit_requested_twice_str, strlen(exit_requested_twice_str) );
    zypper.requestImmediateExit();
  } else {
    write ( STDERR_FILENO, exit_requested_once_str, strlen(exit_requested_once_str) );
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
      mode_t omask = ::umask( 0022 );
      ::umask( omask&0022 );
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
  Zypper & zypper( Zypper::instance() );

  // set up our strings we want to output in case SIGINT was triggered
  // we can not allocate memory there, so we do it in advance and remember the translated strings

  // translators: this will show up if you press ctrl+c once
  std::string exit_requested_once_str  = zypp::str::Format("\n%1%\n") % _("Trying to exit gracefully...");
  ::exit_requested_once_str = exit_requested_once_str.data();

  // translators: this will show up if you press ctrl+c twice
  std::string exit_requested_twice_str = zypp::str::Format("\n%1%\n") % _("Zypper is currently cleaning up, exiting as soon as possible.");
  ::exit_requested_twice_str = exit_requested_twice_str.data();

  if ( ::signal( SIGINT, signal_handler ) == SIG_ERR )
    zypper.out().error("Failed to set SIGINT handler.");
  if ( ::signal (SIGTERM, signal_handler ) == SIG_ERR )
    zypper.out().error("Failed to set SIGTERM handler.");

  if ( ::signal( SIGPIPE, signal_nopipe ) == SIG_ERR )
    zypper.out().error("Failed to set SIGPIPE handler.");

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
    zypper.out().error( e, "Failed to initialize zypper callbacks." );
    report_a_bug( zypper.out() );
    return ZYPPER_EXIT_ERR_BUG;
  }
  catch (...)
  {
    zypper.out().error( "Failed to initialize zypper callbacks." );
    ERR << "Failed to initialize zypper callbacks." << endl;
    report_a_bug( zypper.out() );
    return ZYPPER_EXIT_ERR_BUG;
  }

  int & exitcode { say_goodbye.exitcode };
  exitcode = zypper.main( argc, argv );
  if ( !exitcode )
    exitcode = zypper.exitInfoCode();	// propagate refresh errors even if main action succeeded
  return exitcode;
}
