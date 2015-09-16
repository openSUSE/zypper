/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>
#include <zypp/base/LogTools.h>

#include "Zypper.h"
#include "@command@.h"

///////////////////////////////////////////////////////////////////
// @Command@Options
///////////////////////////////////////////////////////////////////

inline std::ostream & operator<<( std::ostream & str, const @Command@Options & obj )
{ return str << "@Command@Options"; }

///////////////////////////////////////////////////////////////////
namespace
{
  ///////////////////////////////////////////////////////////////////
  /// \class @Command@Impl
  /// \brief Implementation of @command@
  ///////////////////////////////////////////////////////////////////
  class @Command@Impl : public CommandBase<@Command@Impl,@Command@Options>
  {
    typedef CommandBase<@Command@Impl,@Command@Options> CommandBase;
  public:
    @Command@Impl( Zypper & zypper_r ) : CommandBase( zypper_r ) {}
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

  void @Command@Impl::action()
  {
    if ( true )
      throw( Out::Error( ZYPPER_EXIT_ERR_ZYPP, "error", "detail" ) );
    else
      _zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
  }
} // namespace
///////////////////////////////////////////////////////////////////

int @command@( Zypper & zypper_r )
{
  return @Command@Impl( zypper_r ).execute();
}
