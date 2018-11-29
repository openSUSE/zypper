/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "patterns.h"
#include "Zypper.h"
#include "utils/flags/flagtypes.h"
#include "global-settings.h"

using namespace zypp;

PatternsCmdBase::PatternsCmdBase(const std::vector<std::string> &commandAliases_r) :
  ZypperBaseCommand (
    commandAliases_r,
    // translators: command synopsis; do not translate lowercase words
    _("patterns (pt) [OPTIONS] [REPOSITORY] ..."),
    // translators: command summary: patterns, pt
    _("List all available patterns."),
    // translators: command description
    _("List all patterns available in specified repositories."),
    DisableAll
  )
{
  _initRepoFlags.setCompatibilityMode( CompatModeBits::EnableRugOpt | CompatModeBits::EnableNewOpt );
}


zypp::ZyppFlags::CommandGroup PatternsCmdBase::cmdOptions() const
{
  return {};
}

void PatternsCmdBase::doReset()
{ }

int PatternsCmdBase::execute( Zypper &zypper, const std::vector<std::string> & )
{
  list_patterns( zypper, _instOnlyFlags._mode );
  return ZYPPER_EXIT_OK;
}
