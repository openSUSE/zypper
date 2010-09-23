#include <iostream>
#include <signal.h>
//#include <readline/readline.h>

#include "zypp/base/LogTools.h"
#include "zypp/base/LogControl.h"

#include "main.h"
#include "Zypper.h"

#include "callbacks/rpm.h"
#include "callbacks/keyring.h"
#include "callbacks/repo.h"
#include "callbacks/media.h"
#include "callbacks/locks.h"
#include "output/OutNormal.h"
#include "utils/messages.h"

using namespace std;

void signal_handler(int sig)
{
  Zypper & zypper = *Zypper::instance();
  if (zypper.exitRequested())
  {
    /*
    if (zypper.runningShell())
    {
      cout << endl << zypp::str::form(
          _("Use '%s' or enter '%s' to quit the shell."), "Ctrl+D", "quit") << endl;
      ::rl_reset_after_signal();
      exit(ZYPPER_EXIT_ON_SIGNAL);
      //! \todo improve to drop to shell only
    }
    else*/
    {
      // translators: this will show up if you press ctrl+c twice (but outside of zypper shell)
      cerr << endl << _("OK OK! Exiting immediately...") << endl;
      zypper.cleanup();
      exit(ZYPPER_EXIT_ON_SIGNAL);
    }
  }
  else if (zypper.runtimeData().waiting_for_input)
  {
    zypper.cleanup();
    exit(ZYPPER_EXIT_ON_SIGNAL);
  }
  else
  {
    //! \todo cerr << endl << _("Trying to exit gracefully...") << endl;
    zypper.requestExit();
  }
}


int main(int argc, char **argv)
{
  struct Bye {
    ~Bye() {
      MIL << "===== Exiting main() =====" << endl;
    }
  } say_goodbye __attribute__ ((__unused__));

  // set locale
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  // logging
  const char *logfile = getenv("ZYPP_LOGFILE");
  if (logfile == NULL)
    logfile = ZYPPER_LOG;
  zypp::base::LogControl::instance().logfile( logfile );

  MIL << "===== Hi, me zypper " VERSION << endl;
  zypp::dumpRange( MIL, argv, argv+argc, "===== ", "'", "' '", "'", " =====" ) << endl;

  OutNormal out(Out::QUIET);

  if (::signal(SIGINT, signal_handler) == SIG_ERR)
    out.error("Failed to set SIGINT handler.");
  if (::signal(SIGTERM, signal_handler) == SIG_ERR)
    out.error("Failed to set SIGTERM handler.");

  try
  {
    static RpmCallbacks rpm_callbacks;
    static SourceCallbacks source_callbacks;
    static MediaCallbacks media_callbacks;
    static KeyRingCallbacks keyring_callbacks;
    static DigestCallbacks digest_callbacks;
    static LocksCallbacks locks_callbacks;
  }
  catch (const zypp::Exception & e)
  {
    ZYPP_CAUGHT(e);
    out.error(e, "Failed to initialize zypper callbacks.");
    report_a_bug(out);
    return ZYPPER_EXIT_ERR_BUG;
  }
  catch (...)
  {
    out.error("Failed to initialize zypper callbacks.");
    ERR << "Failed to initialize zypper callbacks." << endl;
    report_a_bug(out);
    return ZYPPER_EXIT_ERR_BUG;
  }

  return Zypper::instance()->main(argc, argv);
}
