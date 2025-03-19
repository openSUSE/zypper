﻿/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "installremove.h"
#include "utils/flags/flagtypes.h"
#include "commands/conditions.h"
#include "solve-commit.h"
#include "repos.h"
#include "commonflags.h"

#include <zypp/Pathname.h>
#include <zypp/target/rpm/RpmHeader.h>
#include <yaml-cpp/yaml.h>

using namespace zypp;

//@TODO enable rug compat mode for dry run and --repo

void InstallRemoveBase::fillSrOpts(SolverRequester::Options &sropts_r ) const
{
  sropts_r.force_by_cap  = _selectByCap;
  sropts_r.force_by_name = _selectByName;
}

void InstallRemoveBase::printAndHandleSolverRequesterFeedback( Zypper &zypper, const SolverRequester &sr_r, bool failOnCapNotFound )
{
  sr_r.printFeedback( zypper.out() );

  if ( !zypper.config().ignore_unknown
    && ( sr_r.hasFeedback( SolverRequester::Feedback::NOT_FOUND_NAME )
      || sr_r.hasFeedback( SolverRequester::Feedback::NOT_FOUND_CAP ) ) )
  {
    zypper.setExitCode( ZYPPER_EXIT_INF_CAP_NOT_FOUND );

    //bsc#1127608 Only return with error if zypper is executed non interactively
    //This was missed when migrating the install/remove commands,
    //see commit f2e0f8638a32225ff084458a536eedc0f08ee549
    if ( zypper.config().non_interactive && failOnCapNotFound )
      ZYPP_THROW( ExitRequestException("name or capability not found") );

    return;
  }
  zypper.setExitCode( ZYPPER_EXIT_OK );
}

int InstallRemoveBase::handlePackageFile( Zypper &zypper, std::vector<std::string> &positionalArgs )
{
  const auto &makeError = [ this, &zypper ]( int code, const std::string &msg, const std::string &hint = "" )->int {
    zypper.out().error( msg, hint);
    zypper.out().info( help() );
    return code;
  };

  if ( !_packageFile.empty() ) {
    const auto &info = PathInfo( _packageFile );
    if ( !info.isFile() )
      return makeError( ZYPPER_EXIT_ERR_INVALID_ARGS , _("Packagefile does not point to a valid file") );

    if ( !info.isRUsr() )
      return makeError( ZYPPER_EXIT_ERR_INVALID_ARGS , _("Packagefile is not readable by user") );

    try {
      YAML::Node control = YAML::LoadFile( _packageFile.asString() );

      if ( control.Type() != YAML::NodeType::Sequence ) {
        return makeError( ZYPPER_EXIT_ERR_SYNTAX, "Invalid Packagefile, root node must be a YAML Sequence" );
      }

      for ( const auto &arg : control ) {
        if ( !arg.IsScalar() ) {
          return makeError( ZYPPER_EXIT_ERR_SYNTAX, "Invalid Packagefile, root node sequence can only contain scalars" );
        }
        positionalArgs.push_back( arg.as<std::string>() );
      }
    } catch ( YAML::Exception &e ) {
      return makeError( ZYPPER_EXIT_ERR_SYNTAX, "Invalid Packagefile", e.what() );
    } catch ( ... )  {
      return makeError( ZYPPER_EXIT_ERR_SYNTAX, "Unknown error when parsing the Packagefile" );
    }
  }
  return ZYPPER_EXIT_OK;
}


std::vector<BaseCommandConditionPtr> InstallRemoveBase::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>(),
    std::make_shared<NeedsWritableRoot>()
  };
}

ZyppFlags::CommandGroup InstallRemoveBase::cmdOptions() const
{
  auto that = const_cast<InstallRemoveBase *> ( this );
  return {{
    CommonFlags::resKindSetFlag( that->_kinds ),
    { "name", 'n', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_selectByName, ZyppFlags::StoreTrue, _selectByName ), _("Select packages by plain name, not by capability.") },
    { "capability", 'C', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_selectByCap, ZyppFlags::StoreTrue, _selectByCap ), _("Select packages solely by capability.") },
    CommonFlags::detailsFlag( that->_details ),
    { "packagefile", 'p', ZyppFlags::RequiredArgument , ZyppFlags::PathNameType( that->_packageFile, {}, "FILEPATH" ), "File containing a yaml list of values to be treated as positional arguments." },
    },{
      { "capability", "name" }
    }};
}

void InstallRemoveBase::doReset()
{
  _kinds.clear();
  _details       = false;
  _selectByName  = false;
  _selectByCap   = false;
}

RemoveCmd::RemoveCmd(std::vector<std::string> &&commandAliases_r) :
  InstallRemoveBase (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("remove (rm) [OPTIONS] <CAPABILITY> ..."),
    // translators: command summary: remove, rm
    _("Remove packages."),
    // translators: command description
    _("Remove packages with specified capabilities. A capability is NAME[.ARCH][OP<VERSION>], where OP is one of <, <=, =, >=, >."),
    ResetRepoManager
  )
{
  _dryRun.setCompatibilityMode( CompatModeBits::EnableNewOpt | CompatModeBits::EnableRugOpt );
  _initRepos.setCompatibilityMode( CompatModeBits::EnableNewOpt | CompatModeBits::EnableRugOpt );
}

void RemoveCmd::doReset()
{
  InstallRemoveBase::doReset();
}

int RemoveCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  std::vector<std::string> positionalArgs = positionalArgs_r;

  const auto c = handlePackageFile( zypper, positionalArgs );
  if ( c != ZYPPER_EXIT_OK )
    return c;

  if ( positionalArgs.size() < 1 )
  {
    zypper.out().error(
        _("Too few arguments."),
        _("At least one package name is required."));
    zypper.out().info( help() );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }

  //@TODO refactor this to use all passed kinds instead of just the first one
  //
  ResKind kind = _kinds.size() > 0 ? *_kinds.begin() : ResKind::package;
  // can't remove patch
  if ( kind == ResKind::patch  )
  {
    zypper.out().error( _("Cannot uninstall patches."),
                 _("Installed status of a patch is determined solely based on its dependencies.\n"
                   "Patches are not installed in sense of copied files, database records,\n"
                   "or similar.") );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }

   // can't remove source package
  if ( kind == ResKind::srcpackage )
  {
    zypper.out().error(_("Uninstallation of a source package not defined and implemented."));
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }

  // prepare repositories
  // bsc#606220: don't load repos when removing
  int code = defaultSystemSetup( zypper, InitTarget | LoadTargetResolvables | Resolve );
  if ( code != ZYPPER_EXIT_OK )
    return code;

  // parse package arguments
  PackageArgs::Options argopts;
  argopts.do_by_default = false;
  PackageArgs args( positionalArgs, kind, argopts );

  // tell the solver what we want
  SolverRequester::Options sropts;
  fillSrOpts(sropts);

  SolverRequester sr( sropts );
  sr.remove( args );

  // bsc#980263: relax if removing packages
  // only store exit code but continue with solving
  printAndHandleSolverRequesterFeedback( zypper, sr, false );

  Summary::ViewOptions opts = Summary::DEFAULT;
  if ( _details )
    opts = static_cast<Summary::ViewOptions>( opts | Summary::DETAILS );

  //do solve
  solve_and_commit( zypper, SolveAndCommitPolicy( ).summaryOptions( opts ) );
  return zypper.exitCode();

}

InstallCmd::InstallCmd(std::vector<std::string> &&commandAliases_r) :
  InstallCmd(
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("install (in) [OPTIONS] <CAPABILITY|RPM_FILE_URI> ..."),
    // translators: command summary: install, in
    _("Install packages."),
    // translators: command description
    _("Install packages with specified capabilities or RPM files with specified location. A capability is NAME[.ARCH][OP<VERSION>], where OP is one of <, <=, =, >=, >.")
  )
{}

InstallCmd::InstallCmd( std::vector<std::string> &&commandAliases_r,
                        std::string &&synopsis_r,
                        std::string &&summary_r,
                        CommandDescription &&description_r )
: InstallRemoveBase ( std::move( commandAliases_r ),
                      std::move( synopsis_r ),
                      std::move( summary_r ),
                      std::move( description_r ),
                      ResetRepoManager )
{
  _dryRun.setCompatibilityMode( CompatModeBits::EnableNewOpt | CompatModeBits::EnableRugOpt );
  _initRepos.setCompatibilityMode( CompatModeBits::EnableNewOpt | CompatModeBits::EnableRugOpt );
}

ZyppFlags::CommandGroup InstallCmd::cmdOptions() const
{
  auto that = const_cast<InstallCmd *> ( this );
  ZyppFlags::CommandGroup opts = InstallRemoveBase::cmdOptions();
  opts.options.insert( opts.options.end(), {
    { "from", '\0', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable, ZyppFlags::StringVectorType( &that->_fromRepos, ARG_REPOSITORY ),
      // translators: --from <ALIAS|#|URI>
      _("Select packages from the specified repository.")
    },
    { "oldpackage", '\0', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_oldPackage, ZyppFlags::StoreTrue, _oldPackage ),
      // translators: --oldpackage
      _("Allows one to replace a newer item with an older one. Handy if you are doing a rollback. Unlike --force it will not enforce a reinstall.")
    },
    // disable gpg checks for directly passed rpms
    { "allow-unsigned-rpm", '\0', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_allowUnsignedRPM, ZyppFlags::StoreTrue, _allowUnsignedRPM ),
      _("Silently install unsigned rpm packages given as commandline parameters.")
    },
    { "entire-catalog", '\0', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable | ZyppFlags::Hidden, ZyppFlags::StringVectorType( &that->_entireCatalog, ARG_REPOSITORY ), "" },
    { "force", 'f', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_force, ZyppFlags::StoreTrue, _force ),
      // translators: -f, --force
      _("Install even if the item is already installed (reinstall), downgraded or changes vendor or architecture.")
    },
  });
  opts.conflictingOptions.insert(opts.conflictingOptions.end(),
  {
    { "force", "capability" }
  });
  return opts;
}

void InstallCmd::doReset()
{
  InstallRemoveBase::doReset();
  _force  = false;
  _oldPackage = false;
  _allowUnsignedRPM = false;
  _fromRepos.clear();
  _entireCatalog.clear();
}

int InstallCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  std::vector<std::string> positionalArgs = positionalArgs_r;

  const auto c = handlePackageFile( zypper, positionalArgs );
  if ( c != ZYPPER_EXIT_OK )
    return c;

  if ( positionalArgs.size() < 1 && _entireCatalog.empty() )
  {
    zypper.out().error(
        _("Too few arguments."),
        _("At least one package name is required."));
    zypper.out().info( help() );
    return( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  // rug compatibility code
  if ( !_entireCatalog.empty() )
  {
    if ( !positionalArgs.empty() ) {
      // translators: rug related message, shown if
      // 'zypper in --entire-catalog foorepo someargument' is specified
      zypper.out().warning(_("Ignoring arguments, marking the entire repository."));
    }
    positionalArgs.clear();
    positionalArgs.push_back("*");
    _fromRepos = _entireCatalog;
  }

  // check for rpm files among the arguments
  std::vector<std::string> rpms_files_caps = createTempRepoFromArgs( zypper, positionalArgs, _allowUnsignedRPM );

  // no rpms and no other arguments either
  if ( rpms_files_caps.empty() && positionalArgs.empty() )
  {
    zypper.out().error(_("No valid arguments specified.") );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  int code = defaultSystemSetup( zypper, InitRepos | InitTarget | LoadResolvables | Resolve );
  if ( code != ZYPPER_EXIT_OK )
    return code;

  SolverRequester::Options sropts;
  sropts.force = _force;
  sropts.oldpackage = _oldPackage;
  fillSrOpts(sropts);

  if ( _fromRepos.size() ) {
    int code = repo_specs_to_aliases( zypper, _fromRepos, sropts.from_repos );
    if ( code != ZYPPER_EXIT_OK )
      return code;
  }

  //@TODO refactor this to use all passed kinds instead of just the first one
  //
  ResKind kind = _kinds.size() > 0 ? *_kinds.begin() : ResKind::package;

  // parse package arguments
  PackageArgs args( positionalArgs, kind );

  SolverRequester sr( sropts );
  sr.install( args );

  PackageArgs rpm_args( rpms_files_caps );
  sr.install( rpm_args );

  printAndHandleSolverRequesterFeedback( zypper, sr );

  Summary::ViewOptions opts = Summary::DEFAULT;
  if ( _details )
    opts = static_cast<Summary::ViewOptions>( opts | Summary::DETAILS );

  //do solve
  auto policy = SolveAndCommitPolicy( ).summaryOptions( opts ).downloadMode( _downloadMode.mode() );
  policy.zyppCommitPolicy().allowDowngrade( _oldPackage );
  solve_and_commit( zypper, policy );

  return zypper.exitCode();
}

////////////////////////////////////////////////////////////////////////////////
// RemovePtfCmd
////////////////////////////////////////////////////////////////////////////////

RemovePtfCmd::RemovePtfCmd( std::vector<std::string> &&commandAliases_r )
: InstallCmd( std::move(commandAliases_r),
              // translators: command synopsis; do not translate lowercase words
              _("removeptf (rmptf) [OPTIONS] <PTF|CAPABILITY> ..."),
              // translators: command summary: removeptf, rmptf
              _("Remove (not only) PTFs."),
              // translators: command description
              { _("A remove command which prefers replacing dependant packages to removing them as well. In fact this is a full featured install command with the remove modifier (-) applied to its plain arguments. So 'removeptf foo' is the same as 'install -- -foo'. This is why the command accepts the same options as the install command. It is the recommended way to remove a PTF (Program Temporary Fix)."),
                _("A PTF is typically removed as soon as the fix it provides is applied to the latest official update of the dependant packages. But you don't want the dependant packages to be removed together with the PTF, which is what the remove command would do. The removeptf command however will aim to replace the dependant packages by their official update versions.")
              } )
{}

int RemovePtfCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  static constexpr std::string_view modifiers { "+~-!" }; // explicit install/remove modifiers
  std::vector<std::string> positionalArgs;
  for ( const auto & arg : positionalArgs_r ) {
    if ( modifiers.find( arg[0] ) != std::string_view::npos )
      positionalArgs.push_back( arg );
    else
      positionalArgs.push_back( "-"+arg );
  }
  return InstallCmd::execute( zypper, positionalArgs );
}
