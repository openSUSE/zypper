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

#include "main.h" // for gettext macros

// translator: Option argument like '--export <FILE.repo>'. Do do not translate lowercase wordparts
#define ARG_ALIAS              _( "ALIAS" )
// translator: Option argument like '--export <FILE.repo>'. Do do not translate lowercase wordparts
#define ARG_REOSITORY          _( "ALIAS|#|URI" )
// translator: Option argument like '--export <FILE.repo>'. Do do not translate lowercase wordparts
#define ARG_CATEGORY           _( "CATEGORY" )
// translator: Option argument like '--export <FILE.repo>'. Do do not translate lowercase wordparts
#define ARG_DIR                _( "DIR" )
// translator: Option argument like '--export <FILE.repo>'. Do do not translate lowercase wordparts
#define ARG_FILE               _( "FILE" )
// translator: Option argument like '--export <FILE.repo>'. Do do not translate lowercase wordparts
#define ARG_FILE_repo          _( "FILE.repo" )
// translator: Option argument like '--export <FILE.repo>'. Do do not translate lowercase wordparts
#define ARG_FORMAT             _( "FORMAT" )
// translator: Option argument like '--export <FILE.repo>'. Do do not translate lowercase wordparts
#define ARG_INTEGER            _( "INTEGER" )
// translator: Option argument like '--export <FILE.repo>'. Do do not translate lowercase wordparts
#define ARG_PATH               _( "PATH" )
// translator: Option argument like '--export <FILE.repo>'. Do do not translate lowercase wordparts
#define ARG_SEVERITY           _( "SEVERITY" )
// translator: Option argument like '--export <FILE.repo>'. Do do not translate lowercase wordparts
#define ARG_STRING             _( "STRING" )
// translator: Option argument like '--export <FILE.repo>'. Do do not translate lowercase wordparts
#define ARG_TAG                _( "TAG" )
// translator: Option argument like '--export <FILE.repo>'. Do do not translate lowercase wordparts
#define ARG_TYPE               _( "TYPE" )
// translator: Option argument like '--export <FILE.repo>'. Do do not translate lowercase wordparts
#define ARG_URI                _( "URI" )
// translator: Option argument like '--export <FILE.repo>'. Do do not translate lowercase wordparts
#define ARG_YYYY_MM_DD         _( "YYYY-MM-DD" )

namespace zypp {
namespace ZyppFlags {

  struct CommandOption;

  using DefValueFun = std::function<boost::optional<std::string>()>;
  using SetterFun   = std::function<void ( const CommandOption &, const boost::optional<std::string> &in)>;

  enum ArgFlags : int {
    NoArgument        = 0x00,
    RequiredArgument  = 0x01,
    OptionalArgument  = 0x02,
    ArgumentTypeMask  = 0x0F,

    Repeatable         = 0x10, // < the argument can be repeated
    Hidden             = 0x20, // < command is hidden in help
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
    Value ( DefValueFun &&defValue, SetterFun &&setter, std::string argHint = std::string() );

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
    CommandOption ( std::string &&name_r, char shortName_r, int flags_r, Value &&value_r, std::string &&help_r = std::string());

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
