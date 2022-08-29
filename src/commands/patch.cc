/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "patch.h"
#include "utils/flags/flagtypes.h"
#include "utils/messages.h"
#include "commands/conditions.h"
#include "src/update.h"
#include "solve-commit.h"
#include "SolverRequester.h"
#include "commonflags.h"

#include "Zypper.h"

#include <memory>

PatchCmd::PatchCmd( std::vector<std::string> &&commandAliases_r ) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("patch [OPTIONS]"),
    // translators: command summary: patch
    _("Install needed patches."),
    // translators: command description
    std::string(_("Install all available needed patches."))
    + "\n\n"
    + _("When updating the affected/vulnerable packages described by a patch, zypper always aims for the latest available version."),
    ResetRepoManager
  )
{ }

std::vector<BaseCommandConditionPtr> PatchCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>(),
    std::make_shared<NeedsWritableRoot>()
  };
}

zypp::ZyppFlags::CommandGroup PatchCmd::cmdOptions() const
{
  auto that = const_cast<PatchCmd *>(this);
  return {
    {
      CommonFlags::updateStackOnlyFlag( that->_updateStackOnly),
      { "with-update", '\0', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_withUpdate, ZyppFlags::StoreTrue, _withUpdate ),
            _("Additionally try to update all packages not covered by patches. The option is ignored, if the patch command must update the update stack first. Can not be combined with --updatestack-only.")
      },
      CommonFlags::detailsFlag( that->_details )
    }, {
      { "updatestack-only", "bugzilla" },
      { "updatestack-only", "bz" },
      { "updatestack-only", "cve" },
      { "updatestack-only", "with-update" }
    }
  };
}

void PatchCmd::doReset()
{
  _updateStackOnly = false;
  _withUpdate = false;
  _details = false;
}

int PatchCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  // too many arguments
  if ( !positionalArgs_r.empty() ) {
    report_too_many_arguments( help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  int code = defaultSystemSetup( zypper, InitTarget | InitRepos | LoadResolvables | Resolve );
  if ( code != ZYPPER_EXIT_OK )
    return code;

  // Reset to false when leaving the block in case we are in shell mode!
  DtorReset guard( zypper.runtimeData().solve_with_update );
  if ( _withUpdate )
    zypper.runtimeData().solve_with_update = true;

  bool skip_interactive = _interactiveUpdatesOpts.skipInteractive();
  MIL << "Skipping interactive patches: " << (skip_interactive ? "yes" : "no") << endl;

  SolverRequester::Options srOpts;
  srOpts.skip_interactive = skip_interactive; // bcn #647214
  srOpts.skip_optional_patches = zypper.config().exclude_optional_patches && !_withUpdate;	// bsc#1102261: --with update" should implicitly assume "--with-optional"
  srOpts.cliMatchPatch = CliMatchPatch( zypper,
                                        _selectPatchOpts._select._requestedPatchDates,
                                        _selectPatchOpts._select._requestedPatchCategories,
                                        _selectPatchOpts._select._requestedPatchSeverity );

  // patch --bugzilla/--cve
  if ( _selectPatchOpts._select._requestedIssues.size() ) {
    mark_updates_by_issue( zypper, _selectPatchOpts._select._requestedIssues, srOpts );
  } else {
    SolverRequester sr(srOpts);
    zypper.runtimeData().plain_patch_command = true;
    sr.updatePatches( _updateStackOnly );

    sr.printFeedback( zypper.out() );

    if ( !zypper.config().ignore_unknown
      && ( sr.hasFeedback( SolverRequester::Feedback::NOT_FOUND_NAME )
        || sr.hasFeedback( SolverRequester::Feedback::NOT_FOUND_CAP ) ) )
    {
      zypper.setExitCode( ZYPPER_EXIT_INF_CAP_NOT_FOUND );
      if ( zypper.config().non_interactive )
        ZYPP_THROW( ExitRequestException("name or capability not found") );
    }
  }

  Summary::ViewOptions viewOpts = static_cast<Summary::ViewOptions>( Summary::ViewOptions::DEFAULT | Summary::ViewOptions::SHOW_LOCKS );
  if ( _details ) {
    viewOpts = static_cast<Summary::ViewOptions> ( viewOpts | Summary::ViewOptions::DETAILS );
  }

  if ( _updateStackOnly ) {
    viewOpts = static_cast<Summary::ViewOptions> ( viewOpts | Summary::ViewOptions::UPDATESTACK_ONLY );
  }

  if ( skip_interactive ) {
    // bsc#1183268: Patch reboot needed flag overrules included packages
    viewOpts = static_cast<Summary::ViewOptions> ( viewOpts | Summary::ViewOptions::PATCH_REBOOT_RULES );
  }

  solve_and_commit( zypper, SolveAndCommitPolicy( ).summaryOptions( viewOpts ).downloadMode( _downloadModeOpts.mode() ) );
  return zypper.exitCode();
}
