/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "targetos.h"
#include "utils/flags/flagtypes.h"
#include "Zypper.h"

#include <zypp/Target.h>

using namespace zypp;

TargetOSCmd::TargetOSCmd(const std::vector<std::string> &commandAliases_r) :
  ZypperBaseCommand(
    commandAliases_r,
    // translators: command synopsis; do not translate lowercase words
    _("targetos (tos) [OPTIONS]"),
    // translators: command summary: targetos, tos
    _("Print the target operating system ID string."),
    // translators: command description
    _("Show various information about the target operating system. By default, an ID string is shown."),
    DisableAll
  )
{ }

zypp::ZyppFlags::CommandGroup TargetOSCmd::cmdOptions() const
{
  auto that = const_cast<TargetOSCmd *>(this);
  return {{
    { "label", 'l', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_showOSLabel, ZyppFlags::StoreTrue, _showOSLabel ), _("Show the operating system label.") }
  }};
}

void TargetOSCmd::doReset()
{
  _showOSLabel = false;
}

int TargetOSCmd::execute( Zypper &zypp_r , const std::vector<std::string> & )
{
  if ( zypp_r.out().type() == Out::TYPE_XML )
  {
    zypp_r.out().error(_("XML output not implemented for this command.") );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }

  if ( _showOSLabel )
  {
    if ( zypp_r.globalOpts().terse )
    {
      cout << "labelLong\t" << str::escape(Target::distributionLabel( zypp_r.globalOpts().root_dir ).summary, '\t') << endl;
      cout << "labelShort\t" << str::escape(Target::distributionLabel( zypp_r.globalOpts().root_dir ).shortName, '\t') << endl;
    }
    else
    {
      zypp_r.out().info( str::form(_("Distribution Label: %s"), Target::distributionLabel( zypp_r.globalOpts().root_dir ).summary.c_str() ) );
      zypp_r.out().info( str::form(_("Short Label: %s"), Target::distributionLabel( zypp_r.globalOpts().root_dir ).shortName.c_str() ) );
    }
  }
  else
   zypp_r.out().info( Target::targetDistribution( zypp_r.globalOpts().root_dir ) );

  return ZYPPER_EXIT_OK;
}
