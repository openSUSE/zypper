/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "shell.h"
#include "Zypper.h"
#include "utils/messages.h"

/**
 * @file contains the dummy commands of the zypper shell implementation
 * @todo rewrite shell handling and move the code into this file
 */

ShellCmd::ShellCmd(const std::vector<std::string> &commandAliases_r) :
  ZypperBaseCommand (
    commandAliases_r,
    // translators: command synopsis; do not translate lowercase words
    _("shell (sh)"),
    // translators: command summary: shell, sh
    _("Accept multiple commands at once."),
    // translators: command description
    _("Enter the zypper command shell."),
    DisableAll
  )
{ }

zypp::ZyppFlags::CommandGroup ShellCmd::cmdOptions() const
{
  return {};
}

void ShellCmd::doReset()
{ }

int ShellCmd::execute(Zypper &zypper, const std::vector<std::string> &)
{
  if ( zypper.runningShell() )
    zypper.out().info(_("You already are running zypper's shell.") );
  else
  {
    zypper.out().error(_("Unexpected program flow.") );
    report_a_bug( zypper.out() );
  }
  return zypper.exitCode();
}


ShellQuitCmd::ShellQuitCmd(const std::vector<std::string> &commandAliases_r) :
  ZypperBaseCommand (
    commandAliases_r,
    // translators: command synopsis; do not translate lowercase words
    _("quit (exit, ^D)"),
    // translators: command description
    _("Quit the current zypper shell."),
    // translators: command description
    _("Quit the current zypper shell."),
    DisableAll
  )
{

}

ZyppFlags::CommandGroup ShellQuitCmd::cmdOptions() const
{
  return {};
}

void ShellQuitCmd::doReset()
{ }

int ShellQuitCmd::execute( Zypper &zypper, const std::vector<std::string> & )
{
  if ( !zypper.runningShell() )
    zypper.out().warning(_("This command only makes sense in the zypper shell."), Out::QUIET );
  else
    zypper.out().error( "oops, you wanted to quit, didn't you?" ); // should not happen
  return zypper.exitCode();
}
