/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "zyppflags.h"

#include <getopt.h>
#include <map>
#include <exception>
#include <utility>
#include <string.h>

#include "exceptions.h"

namespace zypp
{

namespace ZyppFlags
{

namespace {

  void appendToOptString ( const CommandOption &option, std::string &optstring )
  {
    if ( option.shortName ) {
      optstring += option.shortName;

      switch ( option.flags & ArgumentTypeMask ) {
        case RequiredArgument:
          optstring += ":";
          break;
        case OptionalArgument:
          optstring += "::";
          break;
        case NoArgument:
          break;
      }
    }
  }

  void appendToLongOptions ( const CommandOption &opt, std::vector<struct option> &opts )
  {
    int has_arg;
    switch ( opt.flags & ArgumentTypeMask ) {
      case NoArgument:
        has_arg = no_argument;
        break;
      case RequiredArgument:
        has_arg = required_argument;
        break;
      case OptionalArgument:
        has_arg = optional_argument;
        break;
    }

    //we do not use the flag and val types, instead we use optind to figure out what happend
    using OptType = struct option;
    opts.push_back(OptType{opt.name.c_str(), has_arg, 0 ,0});
  }
}

Value::Value(DefValueFun &&defValue, SetterFun &&setter, const std::string argHint)
  : _defaultVal( std::move(defValue) ),
    _setter( std::move(setter) ),
    _argHint(argHint)
{

}

void Value::set(const CommandOption &opt, const boost::optional<std::string> in)
{
  if ( _wasSet && !(opt.flags & Repeatable)) {
    ZYPP_THROW( FlagRepeatedException(opt.name) );
  }

  _wasSet = true;

  if ( !in && opt.flags & OptionalArgument ) {
      auto optVal = _defaultVal();
      if (!optVal)
        ZYPP_THROW( ZyppFlagsException(str::Format("BUG: Flag %1% is optional, but no default value was provided") % opt.name) );
      return _setter( opt, optVal );
  } else if ( in || (!in && (opt.flags & ArgumentTypeMask) == NoArgument) )  {
    return _setter(opt, in);
  }

  // this line should never be reached, because the case of required argument is handled directly in parseCLI
  ZYPP_THROW( ZyppFlagsException(str::Format("BUG: Flag %1% requires a value, but non was provided.") % opt.name ) );
}


boost::optional<std::string> Value::defaultValue() const
{
  return _defaultVal();
}

std::string Value::argHint() const
{
  return _argHint;
}

int parseCLI(const int argc, char * const *argv, const std::vector<CommandGroup> &options, const int firstOpt)
{
  // the short options string as used int getopt
  // + - do not permute, stop at the 1st nonoption, which is the command
  // : - return : to indicate missing arg, not ?
  std::string shortopts( "+:" );

  // the set of long options
  std::vector<struct option> longopts;

  //build a complete list and a long and short option index so we can
  //easily get to the CommandOption
  std::vector<CommandOption> allOpts;
  std::map<std::string, int> longOptIndex;  //we do not actually need that index other than checking for dups
  std::map<char, int>        shortOptIndex;

  for ( const CommandGroup &grp : options ) {
    for ( const CommandOption &currOpt : grp.options ) {
      allOpts.push_back( currOpt );

      int allOptIndex = allOpts.size() - 1;

      if ( currOpt.flags & RequiredArgument && currOpt.flags &  OptionalArgument ) {
        throw ZyppFlagsException("Argument can either be Required or Optional");
      }

      if ( !currOpt.name.empty() ) {
        if ( !longOptIndex.insert( { currOpt.name, allOptIndex } ).second) {
          throw ZyppFlagsException("Duplicate long option <insertnamehere>");
        }
        appendToLongOptions( currOpt, longopts );
      }

      if ( currOpt.shortName ) {
        if ( !shortOptIndex.insert( { currOpt.shortName, allOptIndex } ).second) {
          throw ZyppFlagsException("Duplicate short option <insertnamehere>");
        }
        appendToOptString( currOpt, shortopts );
      }
      allOptIndex++;
    }
  }

  //the long options always need to end with a set of zeros
  longopts.push_back({0, 0, 0, 0});

  //setup getopt
  opterr = 0; 			// we report errors on our own
  optind = firstOpt;            // start on the first arg

  while ( true ) {

    int option_index = -1;      //index of the last found long option, same as in allOpts
    int optc = getopt_long( argc, argv, shortopts.c_str(), longopts.data(), &option_index );

    if ( optc == -1 )
      break;

    switch ( optc )
    {
      case '?': {
        // wrong option in the last argument
        // last argument was a short option
        if ( option_index == -1 && optopt)
          ZYPP_THROW(UnknownFlagException( std::string(1, optopt)) );
        else
          ZYPP_THROW(argv[optind - 1]);
        break;
      }
      case ':': {
        //the current argument requires a option
        ZYPP_THROW(MissingArgumentException(argv[optind - 1]));
        break;
      }
      default: {
        int index = -1;
        if ( option_index == -1 ) {
          //we have a short option
          auto it = shortOptIndex.find( (char) optc );
          if ( it != shortOptIndex.end() ) {
            index = it->second;
          }
        } else {
          //we have a long option
          index = option_index;
        }

        if ( index >= 0 ) {

          boost::optional<std::string> arg;
          if ( optarg && strlen(optarg) ) {
            arg = std::string(optarg);
          }

          allOpts[index].value.set( allOpts[index], arg);
        }

        break;
      }
    }
  }
  return optind;
}

void renderHelp(const std::vector<CommandGroup> &options)
{
  for ( const CommandGroup &grp : options ) {
    std::cout << grp.name << ":" << std::endl << std::endl;
    for ( const CommandOption &opt : grp.options ) {

      if ( opt.flags & Hidden )
        continue;

      if ( opt.shortName )
        std::cout << "-" << opt.shortName << ", ";
      else
        std::cout << "    ";

      std::cout << "--" << opt.name;

      std::string argSyntax = opt.value.argHint();
      if ( argSyntax.length() ) {
        if ( opt.flags & OptionalArgument )
          std::cout << "[=";
        else
          std::cout << " <";
        std::cout << argSyntax;
        if ( opt.flags & OptionalArgument )
          std::cout << "]";
        else
          std::cout << ">";
      }

      std::cout << "\t" << opt.help;

      auto defVal = opt.value.defaultValue();
      if ( defVal )
        std::cout << " Default: " << *defVal;

      std::cout << std::endl;

    }
    std::cout<<std::endl;
  }
}

}}

