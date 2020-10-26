/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "ps.h"

#include <iostream>

#include <zypp/base/LogTools.h>
#include <zypp/ExternalProgram.h>
#include <zypp/misc/CheckAccessDeleted.h>

#include "Zypper.h"
#include "Table.h"
#include "utils/messages.h"
#include "utils/flags/flagtypes.h"
#include "commands/needs-rebooting.h"

using namespace zypp;

PSCommand::PSCommand( std::vector<std::string> &&commandAliases_r ) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate the command 'name (abbreviations)' or '-option' names
    _("ps [OPTIONS]"),
    // translators: command description
    _("List running processes which might still use files and libraries deleted by recent upgrades."),
    std::string(),
    DisableAll
  )
{ }

std::string PSCommand::description() const
{
  return summary();
}

ZyppFlags::CommandGroup PSCommand::cmdOptions() const
{
  auto that = const_cast<PSCommand *>(this);
  return {{
    { "short", 's', ZyppFlags::NoArgument | ZyppFlags::Repeatable, ZyppFlags::CounterType( &that->_shortness, _shortness, 3)
             // translators: -s, --short
          ,  _("Create a short table not showing the deleted files. Given twice, show only processes which are associated with a system service. Given three times, list the associated system service names only.")
    }, { "print",  '\0', ZyppFlags::RequiredArgument, ZyppFlags::StringType(&that->_format, boost::optional<const char *>(), "FORMAT")
            // translators: --print <format>
          , _("For each associated system service print <format> on the standard output, followed by a newline. Any '%s' directive in <format> is replaced by the system service name.")
    }, { "debugFile", 'd', ZyppFlags::RequiredArgument, ZyppFlags::StringType(&that->_debugFile, boost::optional<const char *>(), "PATH")
            // translators: -d, --debugFile <path>
          , _("Write debug output to file <path>.")
    }
  }};
}

void PSCommand::doReset()
{
  _shortness = 0;
  _debugFile.clear();
  _format.clear();
}

inline void loadData( CheckAccessDeleted & checker_r )
{
  try
  {
    checker_r.check();
  }
  catch ( const Exception & ex )
  {
    throw( Out::Error( ZYPPER_EXIT_ERR_ZYPP, _("Check failed:"), ex ) );
  }
}

void PSCommand::printServiceNamesOnly()
{
  CheckAccessDeleted checker( false );	// wait for explicit call to check()
  loadData( checker );

  std::set<std::string> services;
  for ( const auto & procInfo : checker )
  {
    std::string service( procInfo.service() );
    if ( ! service.empty() )
      services.insert( std::move(service) );
  }

  const std::string & format( _format );
  if ( format.empty() || format == "%s" )
  {
    for ( const auto & service : services )
    { cout << service << endl; }
  }
  else
  {
    for ( const auto & service : services )
    { cout << str::gsub( format, "%s", service ) << endl; }
  }
}

/**
 * fate #300763
 * Used by 'zypper ps' to show running processes that use
 * libraries or other files that have been removed since their execution.
 * This is particularly useful after 'zypper remove' or 'zypper update'.
 */
int PSCommand::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs )
{
  if ( !positionalArgs.empty() )
  {
    report_too_many_arguments( help() );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }

  // implies -sss
  if ( !_format.empty() )
    _shortness = 3;

  if ( printServiceNamesOnlyEnabled() ) {
    // non table output of service names only
    printServiceNamesOnly();
    return ZYPPER_EXIT_OK;
  }

  // Here: Table output
  zypper.out().info(_("Checking for running processes using deleted libraries..."), Out::HIGH );
  CheckAccessDeleted checker( false );	// wait for explicit call to check()

  if(debugEnabled())
    checker.setDebugOutputFile(_debugFile);

  loadData( checker );

  Table t;
  bool tableWithFiles = tableWithFilesEnabled();
  bool tableWithNonServiceProcs = tableWithNonServiceProcsEnabled();
  t.allowAbbrev(6);
  {
    TableHeader th;
    // process ID
    th << N_("PID")
    // parent process ID
    << N_("PPID")
    // process user ID
    << N_("UID")
    // process login name
    << N_("User")
    // process command name
    << N_("Command")
    // "/etc/init.d/ script that might be used to restart the command (guessed)
    << N_("Service");
    if ( tableWithFiles )
    {
      // "list of deleted files or libraries accessed"
      th << N_("Files");
    }
    t << std::move(th);
  }

  for ( const auto & procInfo : checker )
  {
    std::string service( procInfo.service() );
    if ( ! tableWithNonServiceProcs && service.empty() )
      continue;

    TableRow tr;
    tr << procInfo.pid << procInfo.ppid << procInfo.puid << procInfo.login << procInfo.command << std::move(service);

    if ( tableWithFiles )
    {
      std::vector<std::string>::const_iterator fit = procInfo.files.begin();
      tr << (fit != procInfo.files.end() ? *fit : "");
      t << std::move(tr);

      for ( ++fit; fit != procInfo.files.end(); ++fit )
      { t << ( TableRow() << "" << "" << "" << "" << "" << "" << *fit ); }
    }
    else
    {
      t << std::move(tr);
    }
  }

  if ( t.empty() )
  {
    zypper.out().info(_("No processes using deleted files found.") );
  }
  else
  {
    zypper.out().info(_("The following running processes use deleted files:") );
    cout << endl;
    cout << t << endl;
    zypper.out().info(_("You may wish to restart these processes.") );
    zypper.out().info( str::form( _("See '%s' for information about the meaning of values in the above table."),
                                   "man zypper" ) );
  }

  int exitCode = ZYPPER_EXIT_OK;
  {
    zypper.out().info("");
    exitCode = NeedsRebootingCmd::checkRebootNeeded( zypper, true );
  }

  if ( geteuid() != 0 )
  {
    zypper.out().info("");
    zypper.out().info(_("Note: Not running as root you are limited to searching for files you have permission to examine with the system stat(2) function. The result might be incomplete."));
  }
  return exitCode;
}
