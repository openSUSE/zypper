/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "nullcommands.h"
#include "Zypper.h"
#include "configtest.h"
#include "utils/flags/flagtypes.h"
#include "commands/search/search.h"

MooCmd::MooCmd( std::vector<std::string> &&commandAliases_r ) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("moo"),
    // translators: command summary
    _("Show an animal."),
    // translators: command description
    _("Show an animal."),
    DisableAll
  )
{ }

int MooCmd::execute(Zypper &zypper, const std::vector<std::string> &)
{
  // TranslatorExplanation this is a hedgehog, paint another animal, if you want
  zypper.out().info(_("   \\\\\\\\\\\n  \\\\\\\\\\\\\\__o\n__\\\\\\\\\\\\\\'/_"));
  return ZYPPER_EXIT_OK;
}

WhatProvidesCmd::WhatProvidesCmd( std::vector<std::string> &&commandAliases_r ):
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("what-provides (wp) <CAPABILITY>"),
    // translators: command summary: what-provides, wp
    _("List packages providing specified capability."),
    // translators: command description
    _("List all packages providing the specified capability."),
    DisableAll
  )
{ }

int WhatProvidesCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  // "what-provides" is obsolete
  // The "what-provides" now is included in "search" command, e.g.
  // zypper what-provides 'zypper>1.6'
  // zypper se --match-exact --provides 'zypper>1.6'
  zypper.out().info( str::Format(_("Command '%s' is replaced by '%s'.")) % "what-provides" % "search --provides --match-exact" );
  zypper.out().info( str::Format(_("See '%s' for all available options.")) % "help search" );

  SearchCmd cmd ( this->command() );
  cmd.addRequestedDependency( zypp::sat::SolvAttr::provides );
  cmd.setMode( SearchCmd::MatchMode::Exact );
  cmd.setPositionalArguments( positionalArgs_r );

  return cmd.run( zypper );
}

RupPingCmd::RupPingCmd( std::vector<std::string> &&commandAliases_r ):
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("ping [OPTIONS]"),
    // translators: command summary
    _("This command has dummy implementation which always returns 0."),
    // translators: command description
    _("This command has dummy implementation which always returns 0."),
    DisableAll
  )
{}

int RupPingCmd::execute(Zypper &, const std::vector<std::string> &)
{
  return ZYPPER_EXIT_OK;
}

ZyppFlags::CommandGroup RupPingCmd::cmdOptions() const
{
  return {{
      { "if-active", 'a', ZyppFlags::NoArgument | ZyppFlags::Hidden, ZyppFlags::NoValue(), "" }
    }
  };
}
