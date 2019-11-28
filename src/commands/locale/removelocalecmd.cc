/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "removelocalecmd.h"
#include "utils/flags/flagtypes.h"
#include "utils/messages.h"
#include "commands/commandhelpformatter.h"
#include "commands/conditions.h"
#include "locales.h"

RemoveLocaleCmd::RemoveLocaleCmd(std::vector<std::string> &&commandAliases_r)
  : ZypperBaseCommand (
      std::move( commandAliases_r ),
      // translators: command synopsis; do not translate lowercase words
      _( "removelocale (rloc) [OPTIONS] <LOCALE> ..." ),
      _( "Remove locale(s) from requested locales." ),
      _( "Remove given locale(s) from the list of supported languages." ),
      ResetRepoManager| InitTarget | LoadTargetResolvables
    )
{
  doReset();
}

std::string RemoveLocaleCmd::help()
{
  CommandHelpFormater hlp;
  hlp << ZypperBaseCommand::help();

  hlp.argumentsSection()
    .multiLineText(
      str::form(
         _("Specify locales which shall be removed by the language code. Get the list of requested locales by calling '%s'."), "zypper locales"
      )
    );

  return hlp;
}

zypp::ZyppFlags::CommandGroup RemoveLocaleCmd::cmdOptions() const
{
  auto &that = *const_cast<RemoveLocaleCmd *>(this);
  return {{
    { "no-packages", 'n', ZyppFlags::NoArgument, ZyppFlags::BoolCompatibleType( that._packages, ZyppFlags::StoreFalse ), _("Do not remove corresponding packages for given locale(s).") },
  }};
}

void RemoveLocaleCmd::doReset()
{
  _packages = true;
}

int RemoveLocaleCmd::execute ( Zypper &zypper, const std::vector<std::string> &positionalArgs )
{
  if ( positionalArgs.empty() )
  {
    report_required_arg_missing( zypper.out(), help() );
    return (ZYPPER_EXIT_ERR_INVALID_ARGS);
  }
  removeLocales( zypper, positionalArgs, _packages );
  return zypper.exitCode();
}

std::vector<BaseCommandConditionPtr> RemoveLocaleCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>(),
    std::make_shared<NeedsWritableRoot>()
  };
}
