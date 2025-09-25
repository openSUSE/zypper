/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "removekeycmd.h"
#include "Zypper.h"
#include "keys.h"
#include "utils/messages.h"
#include "utils/flags/flagtypes.h"
#include "commands/conditions.h"

RemoveKeyCmd::RemoveKeyCmd(std::vector<std::string> &&commandAliases_r) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate the command 'name (abbreviations)' or '-option' names
    _("removekey <ID>"),
    // translators: command summary
    _("Remove a key from trust."),
    // translators: command description
    _("Remove key specified by ID from trust."),
    InitTarget )
{
  init();
}

void RemoveKeyCmd::init()
{
  _allMatches = false;
}


zypp::ZyppFlags::CommandGroup RemoveKeyCmd::cmdOptions() const
{
  auto &that = *const_cast<RemoveKeyCmd *>( this );
  return {{
    { "all-matches", '\0', ZyppFlags::NoArgument, ZyppFlags::BoolCompatibleType( that._allMatches ), _("Remove all matched keys.") }
  }};
}

void RemoveKeyCmd::doReset()
{
  init();
}

int RemoveKeyCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs )
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

  removeKey( zypper, positionalArgs.front(), _allMatches );
  return zypper.exitCode();
}

std::vector<BaseCommandConditionPtr> RemoveKeyCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}
