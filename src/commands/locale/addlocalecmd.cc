/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "addlocalecmd.h"
#include "utils/flags/flagtypes.h"
#include "utils/messages.h"
#include "commands/commandhelpformatter.h"
#include "locales.h"

using namespace zypp;

AddLocaleCmd::AddLocaleCmd(std::vector<std::string> &&commandAliases_r)
  : ZypperBaseCommand (
      std::move( commandAliases_r ),
      // translators: command synopsis; do not translate lowercase words
      _( "addlocale (aloc) [OPTIONS] <LOCALE> ..." ),
      _( "Add locale(s) to requested locales." ),
      _( "Add given locale(s) to the list of requested locales." ),
      ResetRepoManager| InitTarget | InitRepos | LoadResolvables
    )
{
  doReset();
}

zypp::ZyppFlags::CommandGroup AddLocaleCmd::cmdOptions() const
{
  auto &that = *const_cast<AddLocaleCmd *>(this);
  return {{
    { "no-packages", 'n', ZyppFlags::NoArgument, ZyppFlags::BoolCompatibleType( that._packages, ZyppFlags::StoreFalse ), _("Do not install corresponding packages for given locale(s).") },
  }};
}

void AddLocaleCmd::doReset()
{
  _packages = true;
}

int AddLocaleCmd::execute(Zypper &zypper, const std::vector<std::string> &positionalArgs )
{
  addLocales( zypper, positionalArgs, _packages );
  return zypper.exitCode();
}


std::string AddLocaleCmd::help()
{
  CommandHelpFormater hlp;
  hlp << ZypperBaseCommand::help();

  hlp.argumentsSection()
    .multiLineText(
      str::form(
        _( "Specify locale which shall be supported by the language code.\n"
           "Get a list of all available locales by calling '%s'."), "zypper locales --all" )
    );

  return hlp;
}
