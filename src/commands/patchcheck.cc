/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "patchcheck.h"
#include "commonflags.h"
#include "utils/flags/flagtypes.h"
#include "utils/messages.h"
#include "src/update.h"

PatchCheckCmd::PatchCheckCmd(const std::vector<std::string> &commandAliases_r) :
  ZypperBaseCommand (
    commandAliases_r,
    // translators: command synopsis; do not translate lowercase words
    _("patch-check (pchk) [OPTIONS]"),
    // translators: command summary: patch-check, pchk
    _("Check for patches."),
    // translators: command description
    _("Display stats about applicable patches. The command returns 100 if needed patches were found, 101 if there is at least one needed security patch."),
    ResetRepoManager
  )
{
  _initReposOpts.setCompatibilityMode( CompatModeBits::EnableNewOpt | CompatModeBits::EnableRugOpt );
}

zypp::ZyppFlags::CommandGroup PatchCheckCmd::cmdOptions() const
{
  auto &that = *const_cast<PatchCheckCmd *>( this );
  return {{
     CommonFlags::updateStackOnlyFlag( that._updateStackOnly )
  }};
}

void PatchCheckCmd::doReset()
{
  _updateStackOnly = false;
}

int PatchCheckCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  // too many arguments
  if ( positionalArgs_r.size() > 0 )
  {
    report_too_many_arguments( help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  int code = defaultSystemSetup( zypper, InitTarget | InitRepos );
  if ( code != ZYPPER_EXIT_OK )
    return code;

  // TODO calc token?

  // now load resolvables:
  code = defaultSystemSetup( zypper, LoadResolvables | Resolve );
  if ( code != ZYPPER_EXIT_OK )
    return code;

  patch_check( _updateStackOnly );
  return zypper.exitCode();
}
