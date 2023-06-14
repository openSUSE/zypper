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

TargetOSCmd::TargetOSCmd(std::vector<std::string> &&commandAliases_r) :
  ZypperBaseCommand(
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("targetos (tos) [OPTIONS]"),
    // translators: command summary: targetos, tos
    _("Print the target operating system ID string."),
    {
      _("Shows the ID string of the target operating system. The string is defined by the XPath:/product/register/target entry in /etc/products.d/baseproduct."),
      _("If the baseproduct does not provide this entry, or if no baseproduct is installed at all, the value is empty if the --terse global option is used."),
      _("In not-terse mode the distribution label is shown instead of an empty value, if a baseproduct is installed."),
    },
    DisableAll
  )
{ }

zypp::ZyppFlags::CommandGroup TargetOSCmd::cmdOptions() const
{
  auto that = const_cast<TargetOSCmd *>(this);
  return {{
    { "label", 'l', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_showOSLabel, ZyppFlags::StoreTrue, _showOSLabel ), _("Show the baseproducts distribution and short label instead.") }
  }};
}

void TargetOSCmd::doReset()
{
  _showOSLabel = false;
}

int TargetOSCmd::execute( Zypper &zypper , const std::vector<std::string> & )
{
  if ( zypper.out().type() == Out::TYPE_XML )
  {
    zypper.out().error(_("XML output not implemented for this command.") );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }

  if ( _showOSLabel )
  {
    if ( zypper.config().terse )
    {
      cout << "labelLong\t" << str::escape(Target::distributionLabel( zypper.config().root_dir ).summary, '\t') << endl;
      cout << "labelShort\t" << str::escape(Target::distributionLabel( zypper.config().root_dir ).shortName, '\t') << endl;
    }
    else
    {
      zypper.out().info( str::form(_("Distribution Label: %s"), Target::distributionLabel( zypper.config().root_dir ).summary.c_str() ) );
      zypper.out().info( str::form(_("Short Label: %s"), Target::distributionLabel( zypper.config().root_dir ).shortName.c_str() ) );
    }
  }
  else {
    std::string targetDistribution { Target::targetDistribution( zypper.config().root_dir ) };
    if ( targetDistribution.empty() ) {
      const Pathname & path { Pathname(zypper.config().root_dir) / "/etc/products.d/baseproduct" };
      // translators: `FILE does not define TAG
      zypper.out().error( str::form(_("%s does not define %s."), path.c_str(), "XPath:/product/register/target" ) );

      if ( not zypper.config().terse ) {
        // Fallback to Distribution Label if not empty...
        targetDistribution = Target::distributionLabel( zypper.config().root_dir ).summary.c_str();
        if ( not targetDistribution.empty() ) {
          targetDistribution = str::form(_("Distribution Label: %s"), targetDistribution.c_str() );
        }
      }
    }
    zypper.out().info( targetDistribution );
  }

  return ZYPPER_EXIT_OK;
}
