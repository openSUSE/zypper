/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "optionsets.h"
#include "utils/flags/flagtypes.h"
#include "global-settings.h"

std::vector<ZyppFlags::CommandGroup> DryRunOptionSet::options()
{
  return {{{
    { "dry-run", 'D', ZyppFlags::NoArgument, ZyppFlags::BoolType( &DryRun::instanceNoConst()._enabled, ZyppFlags::StoreTrue, DryRun::instance()._enabled ),
         _("Don't change anything, just report what would be done.")}
  }}};
}

void DryRunOptionSet::reset()
{
  DryRun::instanceNoConst().reset();
}
