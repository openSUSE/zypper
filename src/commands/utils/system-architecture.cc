/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "system-architecture.h"
#include "utils/flags/flagtypes.h"
#include "Zypper.h"

using namespace zypp;

SystemArchitectureCmd::SystemArchitectureCmd(std::vector<std::string> &&commandAliases_r) :
  ZypperBaseCommand(
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    "system-architecture",
    // translators: command summary: targetos, tos
    _("Print the detected system architecture."),
    {},
    DisableAll
  )
{ }

zypp::ZyppFlags::CommandGroup SystemArchitectureCmd::cmdOptions() const
{ return {}; }

void SystemArchitectureCmd::doReset()
{}

int SystemArchitectureCmd::execute( Zypper &zypper , const std::vector<std::string> & )
{
  zypper.out().info( ZConfig::instance().systemArchitecture().asString() );

  return ZYPPER_EXIT_OK;
}
