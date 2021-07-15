/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "sourceinstall.h"

#include "commands/conditions.h"
#include "utils/flags/flagtypes.h"
#include "repos.h"
#include "misc.h"
#include "global-settings.h"
#include "solve-commit.h"
#include "PackageArgs.h"

SourceInstallCmd::SourceInstallCmd(std::vector<std::string> &&commandAliases_r) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("source-install (si) [OPTIONS] <NAME> ..."),
    // translators: command summary: source-install, si
    _("Install source packages and their build dependencies."),
    // translators: command description
    std::string( _("Install specified source packages and their build dependencies.") ) + "\n\n"+
    (str::Format(_("The default location where rpm installs source packages to is '%1%', but the value can be changed in your local rpm configuration. In case of doubt try executing '%2%'."))
    % "/usr/src/packages/{SPECS,SOURCES}"
    % "rpm --eval \"%{_specdir} and %{_sourcedir}\""
    ).str(),
    ResetRepoManager
  )
{ }

std::vector<BaseCommandConditionPtr> SourceInstallCmd::conditions() const
{
  return {
    std::make_shared<NeedsWritableRoot>()
  };
}

zypp::ZyppFlags::CommandGroup SourceInstallCmd::cmdOptions() const
{
  auto that = const_cast<SourceInstallCmd *>(this);
  return {{
    { "build-deps-only", 'd', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_buildDepsOnly, ZyppFlags::StoreTrue, _buildDepsOnly ),
            // translators: -d, --build-deps-only
            _("Install only build dependencies of specified packages.")
    },
    { "no-build-deps", 'D', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_noBuildDeps, ZyppFlags::StoreTrue, _noBuildDeps ),
            // translators: -D, --no-build-deps
            _("Don't install build dependencies.")
    }
  },{
    //conflicting flags
    { "build-deps-only", "no-build-deps" }
  }};
}

void SourceInstallCmd::doReset()
{
  _buildDepsOnly = false;
  _noBuildDeps   = false;
}

int SourceInstallCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  if ( positionalArgs_r.size() < 1 )
  {
    zypper.out().error(_("Source package name is a required argument.") );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  std::vector<std::string> positionalArgs = positionalArgs_r;
  std::vector<std::string> rpm_caps = createTempRepoFromArgs( zypper, positionalArgs, true );

  if ( rpm_caps.size() )
    positionalArgs.insert( positionalArgs.end(), rpm_caps.begin(), rpm_caps.end() );

  int code = defaultSystemSetup( zypper, InitTarget | InitRepos | LoadResolvables );
  if ( code != ZYPPER_EXIT_OK )
    return code;

  PackageArgs args ( positionalArgs );
  if ( args.donts().size() ) {
    zypper.out().error(_("Uninstalling source packages is not supported.") );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  // no rpms and no other arguments either
  if ( args.dos().empty() ) {
    zypper.out().error(_("No valid arguments specified.") );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  for ( const PackageSpec &spec : args.dos() ) {
    if ( _noBuildDeps ) {
      mark_src_pkgs( zypper, spec );
    } else {
      build_deps_install( zypper, spec, _buildDepsOnly );
    }
  }

  solve_and_commit( zypper, Summary::DEFAULT, _dlOpts.mode() );
  return zypper.exitCode();
}
