/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>

#include <zypp/base/LogTools.h>

#include "Zypper.h"
#include "needs-rebooting.h"

///////////////////////////////////////////////////////////////////
// NeedsRebootingOptions
///////////////////////////////////////////////////////////////////

inline std::ostream & operator<<( std::ostream & str, const NeedsRebootingOptions & obj )
{ return str << "NeedsRebootingOptions"; }

///////////////////////////////////////////////////////////////////
namespace
{
  ///////////////////////////////////////////////////////////////////
  /// \class NeedsRebootingImpl
  /// \brief Implementation of needs-rebooting
  ///////////////////////////////////////////////////////////////////
  class NeedsRebootingImpl : public CommandBase<NeedsRebootingImpl,NeedsRebootingOptions>
  {
    typedef CommandBase<NeedsRebootingImpl,NeedsRebootingOptions> CommandBase;
  public:
    NeedsRebootingImpl( Zypper & zypper_r ) : CommandBase( zypper_r ) {}
    // CommandBase::_zypper
    // CommandBase::options;	// access/manip command options
    // CommandBase::run;	// action + catch and repost Out::Error
    // CommandBase::execute;	// run + final "Done"/"Finished with error" message
    // CommandBase::showHelp;	// Show user help on command
  public:
    /** default action */
    void action();
  };
  ///////////////////////////////////////////////////////////////////

  void NeedsRebootingImpl::action()
  {
    Pathname rebootNeededFlag = Pathname(_zypper.globalOpts().root_dir) / "/run/reboot-needed";

    if ( PathInfo( rebootNeededFlag ).isExist() ) {
      _zypper.out().info( _("Since the last system boot core libraries or services have been updated.") );
      _zypper.out().info( _("Reboot is suggested to ensure that your system benefits from these updates.") );
      _zypper.setExitCode( ZYPPER_EXIT_INF_REBOOT_NEEDED );
    }
    else {
      _zypper.out().info( _("No core libraries or services have been updated since the last system boot.") );
      _zypper.out().info( _("Reboot is probably not necessary.") );
    }
  }
} // namespace
///////////////////////////////////////////////////////////////////

int needsRebooting( Zypper & zypper_r )
{
  return NeedsRebootingImpl( zypper_r ).run();	// Want no final "Done"/"Finished with error." message!
}
