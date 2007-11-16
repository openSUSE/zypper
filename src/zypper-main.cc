#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"

#include "zypper-main.h"
#include "zypper.h"

#define ZYPPER_LOG "/var/log/zypper.log"
#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypper"

using namespace std;

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

  MIL << "Hi, me zypper " VERSION " built " << __DATE__ << " " <<  __TIME__ << endl;

  // parse global options and the command
  int ret = process_globals (argc, argv);
  if (ret != ZYPPER_EXIT_OK)
    return ret;

  switch(command.toEnum())
  {
  case ZypperCommand::SHELL_e:
    command_shell();
    return ZYPPER_EXIT_OK;

  case ZypperCommand::NONE_e:
  {
    if (ghelp)
      return ZYPPER_EXIT_OK;
    else
      return ZYPPER_EXIT_ERR_SYNTAX;
  }

  default:
    return safe_one_command(argc, argv);
  }

  cerr_v << "This line should never be reached." << endl;
  return ZYPPER_EXIT_ERR_BUG;
}
