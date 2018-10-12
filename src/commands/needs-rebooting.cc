/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "needs-rebooting.h"

#include "utils/messages.h"
#include "Zypper.h"

#include <zypp/PathInfo.h>

using namespace zypp;

NeedsRebootingCmd::NeedsRebootingCmd(const std::vector<std::string> &commandAliases_r) :
  ZypperBaseCommand (
    commandAliases_r,
    "needs-rebooting",
    // translators: command summary: needs-rebooting
    _("Check if the needs-reboot flag was set."),
    _("Checks if the needs-reboot flag was set by a previous update or install of a core library or service.\n"
      "Exit code ZYPPER_EXIT_INF_REBOOT_NEEDED indicates that a reboot is needed, otherwise the exit code is set to ZYPPER_EXIT_OK."),
    DisableAll
  )
{}

zypp::ZyppFlags::CommandGroup NeedsRebootingCmd::cmdOptions() const
{
  return zypp::ZyppFlags::CommandGroup();
}

void NeedsRebootingCmd::doReset()
{
  return;
}

int NeedsRebootingCmd::execute( Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r )
{
  if ( !positionalArgs_r.empty() )
  {
    report_too_many_arguments( help() );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }

  filesystem::Pathname rebootNeededFlag = filesystem::Pathname(zypp_r.globalOpts().root_dir) / "/var/run/reboot-needed";

  if ( filesystem::PathInfo( rebootNeededFlag ).isExist() ) {
    zypp_r.out().info( _("Core libraries or services have been updated.") );
    zypp_r.out().info( _("Reboot is required to ensure that your system benefits from these updates.") );
    return ZYPPER_EXIT_INF_REBOOT_NEEDED;
  }
  zypp_r.out().info( _("No core libraries or services have been updated.") );
  zypp_r.out().info( _("Reboot is probably not necessary.") );
  return ZYPPER_EXIT_OK;
}
