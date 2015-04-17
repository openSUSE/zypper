/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#define @COMMAND@
#define @Command@
#define @command@

#ifndef ZYPPER_@COMMAND@_H
#define ZYPPER_@COMMAND@_H

#include <string>
class Zypper;

/*
 @command@ ...
*/

/** @command@ specific options */
struct @Command@ptions : public Options
{
  @Command@Options() : Options( ZypperCommand::@COMMAND@ )
  {}

  /** @Command@ user help (translated). */
  virtual std::ostream & showHelpOn( std::ostream & out ) const;

  bool	_myopt;	//< opts go here...
};

/** Execute @command@.
 */
int @command@( Zypper & zypper_r );

#endif // ZYPPER_@COMMAND@_H
