/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "listupdates.h"
#include "commonflags.h"
#include "utils/messages.h"
#include "src/update.h"

ListUpdatesCmd::ListUpdatesCmd(const std::vector<std::string> &commandAliases_r) :
  ZypperBaseCommand (
    commandAliases_r,
    // translators: command synopsis; do not translate lowercase words
    _("list-updates (lu) [OPTIONS]"),
    // translators: command summary: list-updates, lu
    _("List available updates."),
    // translators: command description
    _("List all available updates."),
    ResetRepoManager
  )
{
  _initReposOpts.setCompatibilityMode( CompatModeBits::EnableNewOpt | CompatModeBits::EnableRugOpt );
}

zypp::ZyppFlags::CommandGroup ListUpdatesCmd::cmdOptions() const
{
  auto &that = *const_cast<ListUpdatesCmd *>(this);
  return {{
    CommonFlags::resKindSetFlag( that._kinds ),
    CommonFlags::bestEffortUpdateFlag( that._bestEffort ),
    { "all", 'a', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that._all, ZyppFlags::StoreTrue, _all ),
          // translators: -a, --all
          _("List all packages for which newer versions are available, regardless whether they are installable or not.")
    }
  }};
}

void ListUpdatesCmd::doReset()
{
  _kinds.clear();
  _all = false;
  _bestEffort = false;
}

int ListUpdatesCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  // too many arguments
  if ( positionalArgs_r.size() > 0 ) {
    report_too_many_arguments( help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  if ( _kinds.empty() )
    _kinds.insert( ResKind::package );

  int code = defaultSystemSetup( zypper, InitTarget | InitRepos | LoadResolvables | Resolve );
  if ( code != ZYPPER_EXIT_OK )
    return code;

  list_updates( zypper, _kinds, _bestEffort, _all );
  return zypper.exitCode();
}
