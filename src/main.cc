#include <iostream>
#include <signal.h>
//#include <readline/readline.h>

#include <zypp/base/LogTools.h>
#include <zypp/base/LogControl.h>

#include "main.h"
#include "Zypper.h"

#include "callbacks/rpm.h"
#include "callbacks/keyring.h"
#include "callbacks/repo.h"
#include "callbacks/media.h"
#include "callbacks/locks.h"
#include "callbacks/job.h"
#include "callbacks/package.h"
#include "output/OutNormal.h"
#include "utils/messages.h"

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

void signal_nopipe( int sig )
{
  WAR << "Exiting on SIGPIPE..." << endl;
  Zypper & zypper( Zypper::instance() );
  zypper.requestImmediateExit();
}

int main( int argc, char **argv )
{
  struct Bye {
    ~Bye() {
      MIL << "===== Exiting main() =====" << endl;
    }
  } say_goodbye __attribute__ ((__unused__));

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
  dumpRange( MIL, argv, argv+argc, "===== ", "'", "' '", "'", " =====" ) << endl;

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
    static PackageCallbacks package_callbacks;
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
  int exitcode = zypper.main( argc, argv );
  if ( !exitcode )
    exitcode = zypper.exitInfoCode();	// propagate refresh errors even if main action succeeded
  return exitcode;
}
