/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPFLAGS_H
#define ZYPPFLAGS_H

#include <string>
#include <functional>
#include <vector>
#include <iostream>
#include <exception>

#include <boost/optional.hpp>

namespace zypp {
namespace ZyppFlags {

  struct CommandOption;

  using DefValueFun = std::function<boost::optional<std::string>()>;
  using SetterFun   = std::function<void ( const CommandOption &, const boost::optional<std::string> &in)>;

  enum ArgFlags : int {
    NoArgument       = 0x00,
    RequiredArgument = 0x01,
    OptionalArgument = 0x02,
    ArgumentTypeMask = 0x0F,

    Repeatable       = 0x10, // < the argument can be repeated
    Hidden           = 0x20  // < command is hidden in help
  };

  /**
   * @class Value
   * Composite type to provide a generic way to write variables and get the default value for them.
   * This type should be only used directly when implementing a new argument type.
   */
  class Value {
  public:
    /**
     * \param defValue takes a functor that returns the default value for the option as string
     * \param setter takes a functor that writes a target variable based on the argument input
     * \param argHint Gives a indicaton what type of data is accepted by the argument
     */
    Value ( DefValueFun &&defValue, SetterFun &&setter, const std::string argHint = std::string() );

    /**
     * Calls the setter functor, with either the given argument or the optional argument
     * if the \a in parameter is null. Additionally it checks if the argument was already seen
     * before and fails if that \a Repeatable flag is not set
     */
    void set( const CommandOption &opt, const boost::optional<std::string> in );

    /**
     * Returns the default value represented as string, or a empty
     * boost::optional if no default value is given
     */
    boost::optional<std::string> defaultValue ( ) const;

    /**
     * returns the hint for the input a command accepts,
     * used in the help
     */
    std::string argHint () const;

  private:
    bool _wasSet = false;
    DefValueFun _defaultVal;
    SetterFun _setter;
    std::string _argHint;
  };

  struct CommandOption
  {
    std::string name;
    char  shortName;
    int flags;
    Value value;
    std::string help;
  };

  struct CommandGroup
  {
    const std::string name;
    std::vector<CommandOption> options;
  };

  /**
   * Parses the command line arguments based on \a options.
   * \returns The first index in argv that was not parsed
   * \throws ZyppFlagsException or any subtypes of it
   */
  int parseCLI ( const int argc, char * const *argv, const std::vector<CommandGroup> &options, const int firstOpt = 1 );

  /**
   * Renders the \a options help string
   */
  void renderHelp( const std::vector<CommandGroup> &options );

}}

#endif // ZYPPFLAGS_H
