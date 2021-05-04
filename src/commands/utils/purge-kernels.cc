/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "purge-kernels.h"
#include "commands/conditions.h"
#include "commands/commonflags.h"
#include "utils/flags/flagtypes.h"
#include "Zypper.h"
#include "solve-commit.h"
#include "global-settings.h"


#include <zypp/Target.h>
#include <zypp/PurgeKernels.h>

using namespace zypp;

PurgeKernelsCmd::PurgeKernelsCmd(std::vector<std::string> &&commandAliases_r) :
  ZypperBaseCommand(
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("purge-kernels [OPTIONS]"),
    // translators: command summary: targetos, tos
    _("Remove old kernels."),
    // translators: command description
    _("Autoremoves installed kernels according to list of kernels to keep from /etc/zypp/zypp.conf:multiversion.kernels which can be given as <version>, latest(-N), running, oldest(+N)."),
    ResetRepoManager | InitTarget | LoadTargetResolvables | Resolve
    )
{ }

zypp::ZyppFlags::CommandGroup PurgeKernelsCmd::cmdOptions() const
{
  auto that = const_cast<PurgeKernelsCmd *>(this);
  return {{
    CommonFlags::detailsFlag( that->_details )
  }};
}

void PurgeKernelsCmd::doReset()
{}

int PurgeKernelsCmd::execute( Zypper &zypper , const std::vector<std::string> & )
{
  PurgeKernels purger;

  zypper.out().gap();
  zypper.out().info( _("Preparing to purge obsolete kernels...") );
  zypper.out().info( str::Format( _("Configuration: %1%") ) % purger.keepSpec() );
  zypper.out().info( str::Format( _("Running kernel release: %1%") ) % purger.unameR() );
  zypper.out().info( str::Format( _("Running kernel arch: %1%") ) % purger.kernelArch().asString() );
  zypper.out().gap();

  purger.markObsoleteKernels();

  Summary::ViewOptions opts = Summary::DEFAULT;
  if ( _details )
    opts = static_cast<Summary::ViewOptions>( opts | Summary::DETAILS );

  zypper.runtimeData().force_resolution = true;
  solve_and_commit( zypper, SolveAndCommitPolicy( ).summaryOptions( opts ) );
  return ZYPPER_EXIT_OK;
}

std::vector<BaseCommandConditionPtr> PurgeKernelsCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>(),
    std::make_shared<NeedsWritableRoot>()
  };
}

