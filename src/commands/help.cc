/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "help.h"
#include "utils/messages.h"
#include "commands/commandhelpformatter.h"
#include "commands/search/search-packages-hinthack.h"
#include "commands/subcommand.h"
#include "Zypper.h"

namespace
{
std::string renderGlobalHelp ()
{
  CommandHelpFormater help;
  help.gMainUsage()

  .gSynopsis( // translators: command synopsis; do not translate lowercase words
  _("zypper [--GLOBAL-OPTIONS] <COMMAND> [--COMMAND-OPTIONS] [ARGUMENTS]")
  )
  .gSynopsis( // translators: command synopsis; do not translate lowercase words
  _("zypper <SUBCOMMAND> [--COMMAND-OPTIONS] [ARGUMENTS]")
  );

  bool first = true;
  for ( const ZyppFlags::CommandGroup & grp : Zypper::instance().configNoConst().cliOptions() ) {
    if ( first ) {
      help.gMainGlobalOpts();
      first = false;
    } else {
      help.gSection( grp.name );
    }
    for ( const ZyppFlags::CommandOption &opt : grp.options ) {
      if ( opt.flags & ZyppFlags::Hidden )
        continue;
      help.gDef( opt.flagDesc( false ), opt.optionHelp() );
    }
  }

  help.gMainCommands();

  for ( const ZypperCommand::CmdDesc &desc : ZypperCommand::allCommands() ) {
    //we stop as soon as we hit the hidden category
    const std::string &cat = std::get<ZypperCommand::CmdDescField::Category>( desc );
    if ( cat == "HIDDEN" )
      break;
    if ( !cat.empty() )
      help.gSection( cat );

    auto cmd = std::get<ZypperCommand::CmdDescField::Factory> ( desc )();
    if ( cmd ) {

      const std::vector<std::string> aliases = cmd->command();

      std::string cmdTxt = aliases.at(0);
      if ( aliases.size() > 1 )
        cmdTxt += ", " + aliases.at(1);

      help.gDef( cmdTxt, cmd->summary() );

      if ( std::get<ZypperCommand::CmdDescField::Id>(desc) == ZypperCommand::SUBCOMMAND_e ) {
	// Get and print the detailed list of available subcommands
	for ( const auto & p : SubCmd::getSubcommandSummaries() ) {
	  help.gDef( p.first, p.second );
	}
      }
    }
  }
  return help;
}
}

HelpCmd::HelpCmd ( std::vector<std::string> &&commandAliases_r ) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    "help",
    _("Print zypper help"),
    _("Print zypper help"),
    DisableAll
  )
{ }


zypp::ZyppFlags::CommandGroup HelpCmd::cmdOptions() const
{
  return {};
}

void HelpCmd::doReset()
{ }

void HelpCmd::printMainHelp( Zypper & zypper )
{
  static std::string globalHelp = renderGlobalHelp();
  zypper.out().info( globalHelp, Out::QUIET );
  print_command_help_hint( zypper );
  return;
}

int HelpCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  if ( positionalArgs_r.size() ) {
    const std::string &cmdArg = positionalArgs_r.at(0);

    try {
      ZypperCommand cmd( cmdArg );

      if ( cmd.toEnum() != ZypperCommand::NONE_e ) {
        ZypperBaseCommand &cmdImpl = cmd.assertCommandObject();
        cmdImpl.reset();
        zypper.out().info( cmdImpl.help(), Out::QUIET );

        if ( cmd.toEnum() == ZypperCommand::SEARCH )
          searchPackagesHintHack::callOrNotify( zypper );

        return ZYPPER_EXIT_OK;
      }
    }
    // exception from command parsing
    catch ( const Exception & e )
    {
      zypper.out().error( e.asUserString() );
      print_unknown_command_hint( zypper, cmdArg );
      return ( ZYPPER_EXIT_ERR_SYNTAX );
    }
  }
  printMainHelp ( zypper );
  return ZYPPER_EXIT_OK;
}
