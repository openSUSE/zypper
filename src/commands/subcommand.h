/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_COMMANDS_SUBCOMMAND_H
#define ZYPPER_COMMANDS_SUBCOMMAND_H

#include "commands/basecommand.h"

#include <zypp/Pathname.h>

#include <vector>
#include <string>
#include <memory>

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
struct SubcommandOptions
{
  /** Subcommand user help (translated).
   * Unlike other commands, this page is created dynamicly for
   * subcommand overview. Command specific help will run
   * 'man zypper-COMMAND' to display the page immediately.
   */
  std::ostream & showHelpOn( std::ostream & out ) const;

public:
  struct Detected
  {
    std::string    _cmd;	///< original command name
    std::string    _name;	///< name of the execuatble
    zypp::Pathname _path;	///< path of the execuatble
  };

  typedef std::vector<std::string> Arglist;


  static const zypp::Pathname _execdir;	///< default location for subcommands

  Detected 	_detected;	///< detected/detectable details
  Arglist	_args;		///< stored command and args (command as _args[0])

  /** Load details about the last subcommand detected by \ref isSubcommand. */
  void loadDetected();

  /** Store new command args */
  template <class Iterator_>
  void args( Iterator_ begin, Iterator_ end )
  { _args = Arglist( begin, end ); }
  /** \overload */
  void args( Arglist args_r )
  { _args = std::move(args_r); }
};


class SubCmd : public ZypperBaseCommand
{
public:
  SubCmd ( const std::vector<std::string> &commandAliases_r, boost::shared_ptr<SubcommandOptions> options_r = boost::shared_ptr<SubcommandOptions>() );

  /** Test whether \c strval_r denotes a subcommand and remember the \ref Detected details.
   * \ref SubcommandOptions can load the last detected details if necessary.
   */
  static bool isSubCommand (const std::string &strval_r );

  /** Execute subcommand (or show its help).
   *
   * \returns 126 subcommand found but not executable
   * \returns 127 subcommand not found
   * \returns subcommands exitCode
   */
  int runCmd( Zypper &zypper );

  boost::shared_ptr<SubcommandOptions>  subCmdOptions ();

private:
  /** Store command in \c _args[0]. */
  void setArg0( std::string arg0_r );

private:
  boost::shared_ptr<SubcommandOptions> _options;

  // ZypperBaseCommand interface
public:
  std::string help() override;

protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute( Zypper &zypper, const std::vector<std::string> &positionalArgs ) override;
};

#endif // ZYPPER_SUBCOMMAND_H
