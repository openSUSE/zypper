/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_SUBCOMMAND_H
#define ZYPPER_SUBCOMMAND_H

#include <vector>
#include <string>
#include <zypp/Pathname.h>

class Zypper;

/*
 subcommand ...

 ZYpper subcommands are standalone executables that live in the
 zypper exec dir, normally /usr/lib/zypper/commands.

 For subcommands zypper provides a wrapper that knows where the
 subcommands live, and runs them by passing command-line arguments
 to them.

 Using zypper global options together with subcommands, as well as
 executing subcommands in zypper shell is currently not supported.

 (If "zypper foo" is not found in the zypper exec dir, the wrapper
  will look in the rest of your $PATH for it. Thus, it’s possible
  to write local zypper extensions that don’t live in system space.)

 (Idea and wording are shamelessly stolen from git :)
*/
/** subcommand specific options */
struct SubcommandOptions : public Options
{
  SubcommandOptions() : Options( ZypperCommand::SUBCOMMAND )
  {}

  /** Subcommand user help (translated).
   * Unlike other commands, this page is created dynamicly for
   * subcommand overview. Command specific help will run
   * 'man zypper-COMMAND' to display the page immediately.
   */
  virtual std::ostream & showHelpOn( std::ostream & out ) const;

public:
  struct Detected
  {
    std::string _cmd;	///< original command name
    std::string _name;	///< name of the execuatble
    Pathname	_path;	///< path of the execuatble
  };

  typedef std::vector<std::string> Arglist;


  static const Pathname _execdir;	///< default location for subcommands

  Detected 	_detected;	///< detected/detectable details
  Arglist	_args;		///< stored command and args (command as _args[0])

  /** Load details about the last subcommand detected by \ref isSubcommand. */
  void loadDetected();

  /** Store new command args */
  template <class Iterator_>
  void args( Iterator_ begin, Iterator_ end )
  { _args = Arglist( begin, end ); }
};

/** Execute subcommand (or show its help).
 *
 * \returns 126 subcommand found but not executable
 * \returns 127 subcommand not found
 * \returns subcommands exitCode
 */
int subcommand( Zypper & zypper_r );

/** Test whether \c strval_r denotes a subcommand and remember the \ref Detected details.
 * \ref SubcommandOptions can load the last detected details if necessary.
 */
bool isSubcommand( const std::string & strval_r );

#endif // ZYPPER_SUBCOMMAND_H
