/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "conditions.h"
#include "Zypper.h"

int NeedsRootCondition::check(std::string &err)
{
  if ( geteuid() != 0 && !Zypper::instance().globalOpts().changedRoot )
  {
    err = _("Root privileges are required to run this command.");
    return ZYPPER_EXIT_ERR_PRIVILEGES;
  }
  return ZYPPER_EXIT_OK;
}
