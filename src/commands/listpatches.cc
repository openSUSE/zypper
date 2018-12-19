/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "listpatches.h"
#include "commonflags.h"
#include "src/update.h"
#include "utils/messages.h"

ListPatchesCmd::ListPatchesCmd(std::vector<std::string> &&commandAliases_r)
  : ZypperBaseCommand (
      std::move( commandAliases_r ),
      // translators: command synopsis; do not translate lowercase words
      _("list-patches (lp) [OPTIONS]"),
      // translators: command summary: list-updates, lu
      _("List available updates."),
      // translators: command description
      _("List all applicable patches."),
      ResetRepoManager
  )
{ }

zypp::ZyppFlags::CommandGroup ListPatchesCmd::cmdOptions() const
{
  auto &that = *const_cast<ListPatchesCmd *>(this);
  return {{
      {"all", 'a', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that._all, ZyppFlags::StoreTrue, _all ),
            // translators: -a, --all
            _("List all patches, not only applicable ones.")
      }
  }};
}

void ListPatchesCmd::doReset()
{
  _all = false;
}

int ListPatchesCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
    // too many arguments
    if ( positionalArgs_r.size() > 0 ) {
      report_too_many_arguments( help() );
      return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
    }

    int code = defaultSystemSetup( zypper, InitTarget | InitRepos | LoadResolvables | Resolve );
    if ( code != ZYPPER_EXIT_OK )
      return code;

    ResKindSet kinds {
      ResKind::patch
    };

    if ( _selectPatchOpts._select._requestedIssues.size() )
      list_patches_by_issue( zypper, _all, _selectPatchOpts._select );
    else
      list_updates( zypper, kinds, false, _all, _selectPatchOpts._select );

    return zypper.exitCode();
}
