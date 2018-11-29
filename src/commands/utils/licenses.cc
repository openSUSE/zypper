/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "licenses.h"
#include "Zypper.h"
#include "misc.h"
#include "utils/messages.h"

using namespace zypp;


LicensesCmd::LicensesCmd( const std::vector<std::string> &commandAliases_r ) :
  ZypperBaseCommand (
    commandAliases_r,
    // translators: command synopsis; do not translate lowercase words
    _("licenses"),
    // translators: command summary: licenses
    _("Print report about licenses and EULAs of installed packages.") ,
    // translators: command description
    _("Report licenses and EULAs of currently installed software packages."),
    DefaultSetup
  )
{ }

ZyppFlags::CommandGroup LicensesCmd::cmdOptions() const
{
  return {};
}

void LicensesCmd::doReset()
{
  return;
}

int LicensesCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  if ( !positionalArgs_r.empty() )
  {
    report_too_many_arguments( help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  report_licenses( zypper );
  return ZYPPER_EXIT_OK;
}
