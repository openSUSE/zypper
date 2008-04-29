#include <iostream>
#include <signal.h>
//#include <readline/readline.h>

#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"

#include "zypper-main.h"
#include "zypper.h"

#include "zypper-rpm-callbacks.h"
#include "zypper-keyring-callbacks.h"
#include "zypper-repo-callbacks.h"
#include "zypper-media-callbacks.h"

using namespace std;

RpmCallbacks rpm_callbacks;
SourceCallbacks source_callbacks;
MediaCallbacks media_callbacks;
KeyRingCallbacks keyring_callbacks;
DigestCallbacks digest_callbacks;


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
      // translators: this will show up if you press ctrl+c twice outside of zypper shell
      cerr << _("OK OK! Exitting immediately...") << endl;
      zypper.cleanup();
      exit(ZYPPER_EXIT_ON_SIGNAL);
    }
  }
  else
    zypper.requestExit();
}


int main(int argc, char **argv)
{
  struct Bye {
    ~Bye() {
      MIL << "Exiting main()" << endl;
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

  if (::signal(SIGINT, signal_handler) == SIG_ERR)
    cerr << "Failed to set SIGINT handler." << endl; 
  if (::signal(SIGTERM, signal_handler) == SIG_ERR)
    cerr << "Failed to set SIGTERM handler." << endl; 

  return Zypper::instance()->main(argc, argv);
}
