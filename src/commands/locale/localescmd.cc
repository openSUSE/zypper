/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "localescmd.h"
#include "utils/flags/flagtypes.h"
#include "commands/commandhelpformatter.h"
#include "locales.h"

LocalesCmd::LocalesCmd( std::vector<std::string> &&commandAliases_r )
  : ZypperBaseCommand (
      std::move( commandAliases_r ),
      // translators: command synopsis; do not translate lowercase words
      _( "locales (lloc) [OPTIONS] [LOCALE] ..." ),
      _( "List requested locales (languages codes)." ),
      str::form ( _( "List requested locales and corresponding packages.\n"
            "\n"
            "Called without arguments, lists the requested locales. If the\n"
            "locale packages for a requested language are not yet on the system, they can\n"
            "be installed by calling '%s'.\n" ), "zypper aloc <LOCALE>" ) ,
      DisableAll

  )
{
  doReset();
}

zypp::ZyppFlags::CommandGroup LocalesCmd::cmdOptions() const
{
  auto &that = *const_cast<LocalesCmd *>(this);
  return {{
    { "packages", 'p', ZyppFlags::NoArgument, ZyppFlags::BoolCompatibleType( that._packages, ZyppFlags::StoreTrue ), _("Show corresponding packages.") },
    { "all", 'a', ZyppFlags::NoArgument, ZyppFlags::BoolCompatibleType( that._all, ZyppFlags::StoreTrue ), _("List all available locales.") }
  }};
}

void LocalesCmd::doReset()
{
  _packages = false;
  _all = false;
}

int LocalesCmd::execute(Zypper &zypper, const std::vector<std::string> &positionalArgs)
{
  if ( _all && positionalArgs.size() ) {
    zypper.out().warning( _("Ignoring positional arguments because --all argument was provided.") );
  }

  SetupSystemFlags flags = ResetRepoManager | InitTarget | LoadTargetResolvables;

  //need to InitRepos if we want to list all, see info about packages or have a search string
  if ( _packages || _all || positionalArgs.size() ) {
    flags = flags | InitRepos | LoadRepoResolvables;
  }

  int code = defaultSystemSetup( zypper, flags  );
  if ( code != ZYPPER_EXIT_OK )
    return code;

  if ( _packages )
    localePackages( zypper, positionalArgs, _all );
  else
    listLocales( zypper, positionalArgs, _all );
  return zypper.exitCode();
}

std::string LocalesCmd::help()
{
  CommandHelpFormater hlp;
  hlp << ZypperBaseCommand::help();
  hlp.argumentsSection()
    .multiLineText(
      _("The locale(s) for which the information shall be printed.")
    )
    .examplesSection()
    .multiLineText(
      _("Get all locales with lang code 'en' that have their own country code, excluding the fallback 'en':")
    )
    .multiLineText(
      "zypper locales 'en_'"
    )
    .gap()
    .multiLineText(
      _("Get all locales with lang code 'en' with or without country code:")
    )
    .multiLineText(
      "zypper locales 'en*'"
    )
    .gap()
    .multiLineText(
      _("Get the locale matching the argument exactly: ")
    )
    .multiLineText(
      "zypper locales 'en'"
    )
    .gap()
    .multiLineText(
      _("Get the list of packages which are available for 'de' and 'en':")
    )
    .multiLineText(
      "zypper locales --packages de en"
    );

  return hlp;
}
