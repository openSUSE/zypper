/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "inrverify.h"
#include "utils/messages.h"
#include "utils/flags/flagtypes.h"
#include "commands/conditions.h"
#include "solve-commit.h"

InrVerifyCmd::InrVerifyCmd( const std::vector<std::string> &commandAliases_r, InrVerifyCmd::Mode cmdMode_r ) :
  ZypperBaseCommand (
    commandAliases_r,
    std::string(),
    std::string(),
    std::string(),
    ResetRepoManager | InitTarget | InitRepos | LoadResolvables
  ),
  _mode ( cmdMode_r )
{
  if ( cmdMode_r == InrVerifyCmd::Verify )
    addOptionSet( _noConfirmOpts );

  _dryRunOpts.setCompatibilityMode( CompatModeBits::EnableNewOpt | CompatModeBits::EnableRugOpt );
}

zypp::ZyppFlags::CommandGroup InrVerifyCmd::cmdOptions() const
{
  auto that = const_cast<InrVerifyCmd *>(this);
  return {{
      { "details", '\0', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_details, ZyppFlags::StoreTrue, _details ),
          // translators: --details
          _("Show the detailed installation summary.")
      }
  }};
}

void InrVerifyCmd::doReset()
{
  _details = false;
}

int InrVerifyCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  // too many arguments
  if ( positionalArgs_r.size() > 0 )
  {
    report_too_many_arguments( help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  Summary::ViewOptions viewOpts = ( Summary::ViewOptions ) Summary::ViewOptions::DEFAULT;
  if ( _details ) {
    viewOpts = ( Summary::ViewOptions ) ( viewOpts | Summary::ViewOptions::DETAILS );
  }

  solve_and_commit( zypper, viewOpts, _downloadOpts.mode() );
  return zypper.exitCode();
}

std::vector<std::string> InrVerifyCmd::synopsis() const
{
  switch ( _mode ) {
  case InrVerifyCmd::InstallRecommends:
    // translators: command synopsis; do not translate lowercase words
    return { _("install-new-recommends (inr) [OPTIONS]") };
  case InrVerifyCmd::Verify:
    // translators: command synopsis; do not translate lowercase words
    return { _("verify (ve) [OPTIONS]") };
  }
  return {};
}

std::string InrVerifyCmd::summary() const
{
  switch ( _mode ) {
  case InrVerifyCmd::InstallRecommends:
    // translators: command summary: install-new-recommends, inr
    return _("Install newly added packages recommended by installed packages.");
  case InrVerifyCmd::Verify:
    // translators: command summary: verify, ve
    return _("Verify integrity of package dependencies.");
  }
  return {};
}

std::string InrVerifyCmd::description() const
{
  switch ( _mode ) {
  case InrVerifyCmd::InstallRecommends:
    // translators: command description
    return _("Install newly added packages recommended by already installed packages. This can typically be used to install new language packages or drivers for newly added hardware.");
  case InrVerifyCmd::Verify:
    // translators: command description
    return _("Check whether dependencies of installed packages are satisfied and suggest to install or remove packages in order to repair the dependency problems.");
  }
  return {};
}

std::vector<BaseCommandConditionPtr> InrVerifyCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>(),
    std::make_shared<NeedsWritableRoot>()
  };
}
