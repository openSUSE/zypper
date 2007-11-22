#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"

#include "zypper-main.h"
#include "zypper.h"

#include "zypper-rpm-callbacks.h"
#include "zypper-keyring-callbacks.h"
#include "zypper-repo-callbacks.h"
#include "zypper-media-callbacks.h"

using namespace std;

ostream no_stream(NULL);

RpmCallbacks rpm_callbacks;
SourceCallbacks source_callbacks;
MediaCallbacks media_callbacks;
KeyRingCallbacks keyring_callbacks;
DigestCallbacks digest_callbacks;

int main(int argc, char **argv)
{
  struct Bye {
    ~Bye() {
      cerr_vv << "Exiting main()" << endl;
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

  Zypper zypper;
  return zypper.main(argc, argv);
}
