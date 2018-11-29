/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "update.h"
#include "conditions.h"
#include "commonflags.h"
#include "solve-commit.h"
#include "SolverRequester.h"
#include "src/update.h"

#include "Zypper.h"

#include <zypp/ZYppFactory.h>

UpdateCmd::UpdateCmd(const std::vector<std::string> &commandAliases_r) :
  ZypperBaseCommand (
    commandAliases_r,
    // translators: command synopsis; do not translate lowercase words
    _("update (up) [OPTIONS] [PACKAGENAME] ..."),
    // translators: command summary: update, up
    _("Update installed packages with newer versions."),
    // translators: command description
    _("Update all or specified installed packages with newer versions, if possible."),
    ResetRepoManager
  )
{
  _initReposOpts.setCompatibilityMode( CompatModeBits::EnableNewOpt | CompatModeBits::EnableRugOpt );
  _licensePolicyOpts.setCompatibilityMode( CompatModeBits::EnableNewOpt | CompatModeBits::EnableRugOpt );
  _dryRunOpts.setCompatibilityMode( CompatModeBits::EnableNewOpt | CompatModeBits::EnableRugOpt );
}

std::vector<BaseCommandConditionPtr> UpdateCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>(),
    std::make_shared<NeedsWritableRoot>()
  };
}

zypp::ZyppFlags::CommandGroup UpdateCmd::cmdOptions() const
{
  auto &that = *const_cast<UpdateCmd *>(this);
  return {{
     CommonFlags::resKindSetFlag( that._kinds ),
     CommonFlags::detailsFlag( that._details ),
     CommonFlags::replaceFilesFlag( that._replaceFiles ),
     CommonFlags::bestEffortUpdateFlag( that._bestEffort )
  }};
}

void UpdateCmd::doReset()
{
  _details = false;
  _replaceFiles = false;
  _bestEffort   = false;
  _kinds.clear();
}

int UpdateCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  //remember if any kind arguments were given on the CLI
  bool kindsOnCLI = _kinds.size();

  if ( ! _kinds.size() )
    _kinds.insert( ResKind::package );
  else {
    if ( _kinds.find( ResKind::product ) != _kinds.end() ) {
      zypper.out().error(_("Operation not supported."),
                  str::form(_("To update installed products use '%s'."),
                            "zypper dup [--from <repo>]") );
      return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
    }

    if ( _kinds.find( ResKind::srcpackage ) != _kinds.end() ) {
      zypper.out().error(_("Operation not supported."),
                  str::form(_("Zypper does not keep track of installed source packages. To install the latest source package and its build dependencies, use '%s'."),
                            "zypper si"));
      return( ZYPPER_EXIT_ERR_INVALID_ARGS );
    }

    if ( !positionalArgs_r.empty() && _kinds.size() > 1 ) {
      zypper.out().error(_("Cannot use multiple types when specific packages are given as arguments.") );
      return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
    }

  }

  int code = defaultSystemSetup( zypper, InitTarget | InitRepos | LoadResolvables | Resolve );
  if ( code != ZYPPER_EXIT_OK )
    return code;

  bool skip_interactive = _interactiveOpts.skipInteractive();
  MIL << "Skipping interactive patches: " << (skip_interactive ? "yes" : "no") << endl;

  SolverRequester::Options sropts;
  sropts.best_effort = _bestEffort;
  sropts.skip_interactive = skip_interactive; // bcn #647214
  sropts.skip_optional_patches = positionalArgs_r.empty() && zypper.globalOpts().exclude_optional_patches;	// without args follow --with[out]-optional

  SolverRequester sr(sropts);
  if ( positionalArgs_r.empty() )
  {
    for( const ResKind &kind : _kinds )
    {
      if ( kind == ResKind::package )
      {
        MIL << "Computing package update..." << endl;
        // this will do a complete package update as far as possible
        // while respecting solver policies
        zypp::getZYpp()->resolver()->doUpdate();
        // no need to call Resolver::resolvePool() afterwards
        zypper.runtimeData().solve_before_commit = false;
      }
      // update -t patch; patch
      else if ( kind == ResKind::patch )
      {
        zypper.runtimeData().plain_patch_command = true;
        sr.updatePatches( false );
      }
      else if ( kind == ResKind::pattern )
        sr.updatePatterns();
      // should not get here (see above kind parsing code), but just in case
      else
      {
        zypper.out().error(_("Operation not supported.") );
        return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
      }
    }
  }
  // update with arguments
  else {
    //only one type argument is allowed if arguments are given, check is above
    PackageArgs args( positionalArgs_r, *_kinds.begin() );
    sr.update( args );
  }

  sr.printFeedback( zypper.out() );

  if ( !zypper.globalOpts().ignore_unknown
    && ( sr.hasFeedback( SolverRequester::Feedback::NOT_FOUND_NAME )
      || sr.hasFeedback( SolverRequester::Feedback::NOT_FOUND_CAP ) ) )
  {
    zypper.setExitCode( ZYPPER_EXIT_INF_CAP_NOT_FOUND );
    if ( zypper.globalOpts().non_interactive )
      ZYPP_THROW( ExitRequestException("name or capability not found") );
  }

  Summary::ViewOptions viewOpts = static_cast<Summary::ViewOptions>( Summary::ViewOptions::DEFAULT | Summary::ViewOptions::SHOW_LOCKS );
  if ( _details ) {
    viewOpts = static_cast<Summary::ViewOptions> ( viewOpts | Summary::ViewOptions::DETAILS );
  }

  // show not updated packages if 'zypper up' (-t package or -t product)
  if ( positionalArgs_r.empty()
       && ( !kindsOnCLI
            || _kinds.find( ResKind::package ) != _kinds.end()
            || _kinds.find( ResKind::product ) != _kinds.end() ) ) {
    viewOpts = static_cast<Summary::ViewOptions> ( viewOpts | Summary::SHOW_NOT_UPDATED );
  }

  solve_and_commit( zypper, viewOpts, _downloadModeOpts.mode() );
  return zypper.exitCode();
}
