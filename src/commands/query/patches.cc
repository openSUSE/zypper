/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "patches.h"
#include "Zypper.h"
#include "utils/flags/flagtypes.h"
#include "global-settings.h"

using namespace zypp;

PatchesCmdBase::PatchesCmdBase(std::vector<std::string> &&commandAliases_r) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("patches (pch) [REPOSITORY] ..."),
    // translators: command summary: patches, pch
    _("List all available patches."),
    // translators: command description
    _("List all patches available in specified repositories."),
    DisableAll
  )
{
  _initRepoFlags.setCompatibilityMode( CompatModeBits::EnableRugOpt | CompatModeBits::EnableNewOpt );
}


zypp::ZyppFlags::CommandGroup PatchesCmdBase::cmdOptions() const
{
  return {};
}

void PatchesCmdBase::doReset()
{ }

int PatchesCmdBase::execute( Zypper &zypper, const std::vector<std::string> & )
{
  list_patches( zypper );
  return ZYPPER_EXIT_OK;
}
