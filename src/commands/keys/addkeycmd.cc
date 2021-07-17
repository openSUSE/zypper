/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "addkeycmd.h"
#include "Zypper.h"
#include "keys.h"
#include "commands/conditions.h"
#include "utils/messages.h"

AddKeyCmd::AddKeyCmd(std::vector<std::string> &&commandAliases_r) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate the command 'name (abbreviations)' or '-option' names
    _("addkey <URL>"),
    // translators: command summary
    _("Add a new key to trust."),
    // translators: command description
    _("Imports a new key into the trusted database, the file can be specified as a URL."),
    InitTarget )
{ }


zypp::ZyppFlags::CommandGroup AddKeyCmd::cmdOptions() const
{
  return {};
}

void AddKeyCmd::doReset()
{
  return;
}

int AddKeyCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs )
{
  // too many arguments
  if ( positionalArgs.size() > 1 ) {
    report_too_many_arguments( help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  //not enough args
  if ( positionalArgs.size() < 1 ) {
    report_too_few_arguments( help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  importKey( zypper, positionalArgs.front() );
  return zypper.exitCode();
}


std::vector<BaseCommandConditionPtr> AddKeyCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}
