/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_NEEDS_REBOOTING_H
#define ZYPPER_NEEDS_REBOOTING_H

#include <string>
class Zypper;

/*
      "needs-rebooting\n"
      "\n"
      "Checks if the reboot-needed flag was set by a previous update or install of\n"
      "a core library or service.\n"
      "Exit code ZYPPER_EXIT_INF_REBOOT_NEEDED indicates that a reboot is suggested,\n"
      "otherwise the exit code is set to ZYPPER_EXIT_OK.\n"
      "\n"
      "This is the recommended way for scripts to test whether a system reboot is\n"
      "suggested.\n"
 */

/** needs-rebooting specific options */
struct NeedsRebootingOptions : public Options
{
  NeedsRebootingOptions()
  : Options( ZypperCommand::NEEDS_REBOOTING )
  {}
};

/** Execute needs-rebooting.
 */
int needsRebooting( Zypper & zypper_r );

#endif // ZYPPER_NEEDS_REBOOTING_H
