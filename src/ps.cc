/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>

#include <zypp/base/LogTools.h>
#include <zypp/ExternalProgram.h>
#include <zypp/misc/CheckAccessDeleted.h>

#include "Zypper.h"
#include "Table.h"
#include "ps.h"

///////////////////////////////////////////////////////////////////
// PsOptions
///////////////////////////////////////////////////////////////////

inline std::ostream & operator<<( std::ostream & str, const PsOptions & obj )
{ return str << "PsOptions"; }

///////////////////////////////////////////////////////////////////
namespace
{
  ///////////////////////////////////////////////////////////////////
  /// \class PsImpl
  /// \brief Implementation of ps
  ///////////////////////////////////////////////////////////////////
  class PsImpl : public CommandBase<PsImpl,PsOptions>
  {
    typedef CommandBase<PsImpl,PsOptions> CommandBase;
  public:
    PsImpl( Zypper & zypper_r ) : CommandBase( zypper_r ) {}
    // CommandBase::_zypper
    // CommandBase::options;	// access/manip command options
    // CommandBase::run;	// action + catch and repost Out::Error
    // CommandBase::execute;	// run + final "Done"/"Finished with error" message
    // CommandBase::showHelp;	// Show user help on command
  public:
    /** default action */
    void action();

  private:
    void printServiceNamesOnly();
  };
  ///////////////////////////////////////////////////////////////////

  inline void loadData( zypp::CheckAccessDeleted & checker_r )
  {
    try
    {
      checker_r.check();
    }
    catch ( const zypp::Exception & ex )
    {
      throw( Out::Error( ZYPPER_EXIT_ERR_ZYPP, _("Check failed:"), ex ) );
    }
  }

  void PsImpl::printServiceNamesOnly()
  {
    zypp::CheckAccessDeleted checker( false );	// wait for explicit call to check()
    loadData( checker );

    std::set<std::string> services;
    for ( const auto & procInfo : checker )
    {
      std::string service( procInfo.service() );
      if ( ! service.empty() )
	services.insert( std::move(service) );
    }

    const std::string & format( options()._format );
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
  void PsImpl::action()
  {
    if ( options().printServiceNamesOnly() )
    {
      // non table output of service names only
      return printServiceNamesOnly();
    }

    // Here: Table output
    _zypper.out().info(_("Checking for running processes using deleted libraries..."), Out::HIGH );
    zypp::CheckAccessDeleted checker( false );	// wait for explicit call to check()
    loadData( checker );

    Table t;
    bool tableWithFiles = options().tableWithFiles();
    bool tableWithNonServiceProcs = options().tableWithNonServiceProcs();
    t.allowAbbrev(6);
    {
      TableHeader th;
      // process ID
      th << _("PID")
      // parent process ID
      << _("PPID")
      // process user ID
      << _("UID")
      // process login name
      << _("User")
      // process command name
      << _("Command")
      // "/etc/init.d/ script that might be used to restart the command (guessed)
      << _("Service");
      if ( tableWithFiles )
      {
	// "list of deleted files or libraries accessed"
	th << _("Files");
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
      _zypper.out().info(_("No processes using deleted files found.") );
    }
    else
    {
      _zypper.out().info(_("The following running processes use deleted files:") );
      cout << endl;
      cout << t << endl;
      _zypper.out().info(_("You may wish to restart these processes.") );
      _zypper.out().info( str::form( _("See '%s' for information about the meaning of values in the above table."),
				     "man zypper" ) );
    }

    if ( geteuid() != 0 )
    {
      _zypper.out().info("");
      _zypper.out().info(_("Note: Not running as root you are limited to searching for files you have permission to examine with the system stat(2) function. The result might be incomplete."));
    }
  }
} // namespace
///////////////////////////////////////////////////////////////////

int ps( Zypper & zypper_r )
{
  return PsImpl( zypper_r ).run();	// no final "Done"/"Finished with error." message!
}
