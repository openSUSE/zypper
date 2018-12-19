#ifndef ZYPPER_COMMANDS_BASECOMMAND_INCLUDED
#define ZYPPER_COMMANDS_BASECOMMAND_INCLUDED

#include <vector>
#include <string>
#include <memory>

#include <zypp/base/Flags.h>
#include "utils/flags/zyppflags.h"

#include <zypp/ZYpp.h>

class Zypper;
class ZypperBaseCommand;

/**
 * Use this type as baseclass for all commonly used
 * Option Sets, e.g. SolverOptions. By passing a pointer to the \a parent
 * \sa ZypperBaseCommand they are automatically added to the commands option
 * list.
 */
class BaseCommandOptionSet
{
public:
  BaseCommandOptionSet();
  BaseCommandOptionSet(ZypperBaseCommand &parent );
  virtual ~BaseCommandOptionSet();
  virtual std::vector<zypp::ZyppFlags::CommandGroup> options () = 0;
  virtual void reset () = 0;
};

/**
 * Describes and checks a prerequisite for a command to be executed,
 * those are returned by \sa ZypperBaseCommand::conditions and will
 * be checked before \sa ZypperBaseCommand::run is called.
 * A failing check will exit zypper.
 */
class BaseCommandCondition
{
public:
  virtual ~BaseCommandCondition();
  virtual int check ( std::string &err ) = 0;
};
using BaseCommandConditionPtr = std::shared_ptr<BaseCommandCondition>;

/** Flags for tuning \ref ZypperBaseCommand::defaultSystemSetup. */
enum SetupSystemBits
{
 DisableAll             = 0,
 ResetRepoManager       = (1 << 1),             //< explicitely reset the repomanager before calling the cmd
 InitTarget		= (1 << 2),		//< Initialize the target
 InitRepos		= (1 << 3),		//< Initialize repositories
 NoSystemResolvables    = (1 << 4),             //< Disable the loading of system resolvables
 LoadResolvables        = (1 << 5),             //< Load resolvables
 Resolve                = (1 << 6),             //< compute status of PPP
 DefaultSetup           = ResetRepoManager | InitTarget | InitRepos | LoadResolvables | Resolve
};
ZYPP_DECLARE_FLAGS( SetupSystemFlags, SetupSystemBits );

/** \relates LoadSystemFlags */
ZYPP_DECLARE_OPERATORS_FOR_FLAGS( SetupSystemFlags );




/**
 * All Zypper commands should derive from this type. It automatically
 * handles option building and makes it possible to keep the commands
 * seperated from each other.
 */
class ZypperBaseCommand
{
public:
  friend class BaseCommandOptionSet;

  ZypperBaseCommand ( std::vector<std::string> &&commandAliases_r,
                      std::string &&synopsis_r,
                      std::string &&summary_r = std::string(),
                      std::string &&description_r = std::string(),
                      SetupSystemFlags systemInitFlags_r = DefaultSetup
  );

  ZypperBaseCommand ( std::vector<std::string> &&commandAliases_r,
                      std::vector<std::string> &&synopsis_r,
                      std::string &&summary_r = std::string(),
                      std::string &&description_r = std::string(),
                      SetupSystemFlags systemInitFlags_r = DefaultSetup
  );

  virtual ~ZypperBaseCommand();

  /**
   * Called right before option parsing, this will reset all
   * options to default.
   * The default implemention calls \sa doReset and resets
   * all registered \sa BaseCommandOptionSet instances
   */
  virtual void reset ();

  /**
   * Returns a list of command aliases that are accepted
   * on the command line
   */
  virtual std::vector<std::string> command () const;

  /**
   * Returns the command summary, a one line description
   * of what it does. Used in "zypper help" command overview.
   * \sa help
   */
  virtual std::string summary () const;

  /**
   * Returns the synopsis of how the command is to be called.
   * \example "list (ll) [options]"
   * \sa help
   */
  virtual std::vector<std::string> synopsis () const;

  /**
   * Returns a short description of what the command does, this is
   * used in the commands help page.
   * \sa help
   */
  virtual std::string description () const;

  /**
   * Initialize the command options and positional arguments for the command
   * \throws ZyppFlags::ZyppFlagsException
   */
  int parseArguments ( Zypper &zypper, const int firstOpt );


  /**
   * Prepares the command to be executed and checks all conditions before
   * calling \sa execute.
   */
  int run ( Zypper &zypper );

  /**
   * The default implementation uses \a synopsis , \a description and
   * \a options to compile the help text.
   */
  virtual std::string help ();

  /**
   * Returns the list of all supported options, including all options
   * from registered option sets and a automatically inserted help option.
   * \sa addOptionSet
   */
  std::vector<zypp::ZyppFlags::CommandGroup> options ();

  /**
   * Returns true if the help flag was set on command line
   */
  bool helpRequested () const;

  /**
   * Returns the system setup flags set by the constructor
   * \sa systemSetup
   * \sa defaultSystemSetup
   */
  SetupSystemFlags setupSystemFlags () const;

  zypp::ZYpp::Ptr zyppApi () const;

  /**
   * Returns the list of positional arguments that was parsed from raw cli args
   * \sa BaseCommand::parseArguments
   */
  std::vector<std::string> positionalArguments() const;

  /**
   * Directly sets the list of positional arguments, this is supposed to be used
   * when chaining commands, and directly setting options
   * \sa BaseCommand::parseArguments
   */
  void setPositionalArguments(const std::vector<std::string> &positionalArguments);

  /**
   * Returns the list of raw options that was parsed from raw cli args.
   * \note this is only enabled if \sa setFillRawOptions was set to true
   * \sa BaseCommand::parseArguments
   * \sa BaseCommand::fillRawOptions
   * \sa BaseCommand::setFillRawOptions
   */
  std::vector<std::string> rawOptions() const;

  /**
   * Returns if filling raw options is enabled when parsing arguments
   * \sa BaseCommand::parseArguments
   */
  bool fillRawOptions() const;

  /**
   * Enables or disables filling of raw options when parsing arguments
   * \sa BaseCommand::parseArguments
   * \note this is diabled by default
   */
  void setFillRawOptions(bool fillRawOptions);

protected:
  /**
   * Registers a option set to be supported on command line
   * \sa BaseCommandOptionSet
   */
  void addOptionSet ( BaseCommandOptionSet &set );

  /**
   * Returns the list of conditions that need to be met in order to execute
   * the command. Failing a condition will exit zypper.
   */
  virtual std::vector<BaseCommandConditionPtr> conditions() const;

  /**
   * Reimplement to return the commands own options.
   */
  virtual zypp::ZyppFlags::CommandGroup cmdOptions () const = 0;

  /**
   * Reimplement to reset all options to the default
   * values before option parsing
   */
  virtual void doReset () = 0;

  /**
   * Reimplement to define
   */
  virtual int execute ( Zypper &zypper, const std::vector<std::string> &positionalArgs ) = 0;

  /**
   * Sets up the system before the command is executed, reimplement to change default behaviour.
   * The default implementation calls \sa defaultSystemSetup
   */
  virtual int systemSetup (Zypper &zypper );

  /**
   * Initializes the system according to bits set in \a flags
   */
  int defaultSystemSetup(Zypper &zypper, SetupSystemFlags flags_r = DefaultSetup );

private:
  std::vector<BaseCommandOptionSet *> _registeredOptionSets;
  bool _helpRequested = false;
  bool _fillRawOptions = false;
  std::vector<std::string> _commandAliases;
  std::vector<std::string> _synopsis;
  std::vector<std::string> _positionalArguments;
  std::vector<std::string> _rawOptions;
  std::string _summary;
  std::string _description;
  SetupSystemFlags _systemInitFlags;
};

using ZypperBaseCommandPtr = std::shared_ptr<ZypperBaseCommand> ;

#endif


