/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "zyppflags.h"
#include "flagtypes.h"
#include "exceptions.h"
#include "utils/messages.h"
#include "Zypper.h"

#include <getopt.h>
#include <unordered_map>
#include <exception>
#include <utility>
#include <string.h>
#include <algorithm>

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
      default:
        throw ZyppFlagsException( str::Format("Invalid opt.flags value for option: %1%") % opt.nameStr() );
    }

    //we do not use the flag and val types, instead we use optind to figure out what happend
    using OptType = struct option;
    opts.push_back( OptType{opt.name.c_str(), has_arg, 0 ,0} );
  }
}

Value::Value( DefValueFun &&defValue, SetterFun &&setter, std::string argHint )
  : _defaultVal( std::move(defValue) ),
    _setter( std::move(setter) ),
    _argHint( std::move(argHint) )
{

}

void Value::set( const CommandOption &opt, const boost::optional<std::string> in )
{
  if ( _wasSet && !(opt.flags & Repeatable) ) {
    // bsc#1123865: don't throw, just warn
    Zypper::instance().out().warning( FlagRepeatedException(opt.nameStr()).asString() );
    return;
  }

  if ( _preWriteHook.size() ) {
    for ( auto &hook : _preWriteHook ) {
      if ( !hook ( opt, in ) )
        return;
    }
  }

  _wasSet = true;

  bool runPostSetHook = false;
  if ( !in && opt.flags & OptionalArgument ) {
      auto optVal = _defaultVal();
      if ( !optVal )
        ZYPP_THROW( ZyppFlagsException( str::Format("BUG: Flag %1% is optional, but no default value was provided") % opt.nameStr() ) );
      _setter( opt, optVal );
      runPostSetHook = true;
  } else if ( in || ( !in && ( opt.flags & ArgumentTypeMask ) == NoArgument ) )  {
    _setter( opt, in );
    runPostSetHook = true;
  }

  if ( runPostSetHook ) {
    if ( _postWriteHook.size() ) {
      for ( auto &hook : _postWriteHook ) {
        hook ( opt, in );
      }
    }
    return;
  }

  // this line should never be reached, because the case of required argument is handled directly in parseCLI
  ZYPP_THROW( ZyppFlagsException(str::Format("BUG: Flag %1% requires a value, but non was provided.") % opt.nameStr() ) );
}

void Value::neverUsed()
{
  if ( _notFoundHook )
    _notFoundHook();
}

boost::optional<std::string> Value::defaultValue() const
{
  return _defaultVal();
}

std::string Value::argHint() const
{
  return _argHint;
}

bool Value::wasSet() const
{
  return _wasSet;
}

Value &Value::after( std::function<void ()> &&postWriteHook )
{
  return after ( [ postWriteHook ] ( const CommandOption &, const boost::optional<std::string> & ) {
    postWriteHook();
  } );
}

Value &Value::notSeen( std::function<void ()> &&notFoundHook )
{
  _notFoundHook = notFoundHook;
  return *this;
}

Value & Value::before( PreWriteHook &&preWriteHook )
{
  _preWriteHook.push_back( std::move(preWriteHook) );
  return *this;
}

Value & Value::after( PostWriteHook &&postWriteHook )
{
  _postWriteHook.push_back( std::move(postWriteHook) );
  return *this;
}


int parseCLI( const int argc, char * const *argv, const std::vector<CommandGroup> &options )
{
  // the short options string as used int getopt
  // + - do not permute, stop at the 1st nonoption, which is the command
  // : - return : to indicate missing arg, not ?
  std::string shortopts( "+:" );

  // the set of long options
  std::vector<struct option> longopts;

  // the set of all conflicting options
  ConflictingFlagsList conflictingFlags;

  //build a complete list and a long and short option index so we can
  //easily get to the CommandOption
  std::vector<CommandOption> allOpts;
  std::unordered_map<std::string, int> longOptIndex;  //we do not actually need that index other than checking for dups
  std::unordered_map<char, int>        shortOptIndex;

  //we intentionally copy all the CommandOptions here instead of using pointers, each state change in the
  //Options or Value implementations are only valid for one run of parseCLI, state should not be remembered
  //to not influence a possible next run
  for ( const CommandGroup &grp : options ) {
    for ( const CommandOption &currOpt : grp.options ) {
      allOpts.push_back( currOpt );

      int allOptIndex = allOpts.size() - 1;
      int flags = currOpt.flags;

      if ( flags & RequiredArgument && flags &  OptionalArgument ) {
        throw ZyppFlagsException("Argument can either be Required or Optional");
      }

      if ( !currOpt.name.empty() ) {
        if ( !longOptIndex.insert( { currOpt.name, allOptIndex } ).second ) {
          throw ZyppFlagsException( str::Format("Duplicate long option %1%") % currOpt.nameStr() );
        }
        appendToLongOptions( currOpt, longopts );
      }

      if ( currOpt.shortName ) {
        if ( !shortOptIndex.insert( { currOpt.shortName, allOptIndex } ).second ) {
          throw ZyppFlagsException( str::Format("Duplicate short option %1%") % currOpt.shortNameStr() );
        }
        appendToOptString( currOpt, shortopts );
      }
    }
    conflictingFlags.insert( conflictingFlags.end(), grp.conflictingOptions.begin(), grp.conflictingOptions.end() );
  }

  //the long options always need to end with a set of zeros
  longopts.push_back( {0, 0, 0, 0} );



  /*
   * Because of complex rules for flags in zypper, its required to process
   * them in a defined order. We model the dependency tree in a very simple
   * list of options with their first level dependencies, then use a multi pass
   * approach to find directly writeable flags.
   *
   * A flag is considered directly writeable if it either has no dependencies,
   * or all dependencies are solved already.
   * We keep track of solved flags in a simple set.
   *
   * Consider the following dependency tree (read from top down):
   *
   *     A      D
   *    / \    / \
   *   B   \  E  F
   *        \/
   *        c
   *
   * The list representation would look like that:
   *
   *   |-A-B
   *   |  |_C
   *   |-B
   *   |-C
   *   |-D-E
   *   |  |_F
   *   |-E-C
   *   |-F
   *
   * The first pass through will write
   *    B, C, and F
   * Second pass through
   *    A, E
   * Third pass through
   *    D
   *
   * We keep track of the number of values to be written, if the number
   * does not change during a pass through something in the dependency chain is broken
   * we need to give up, since its most likely to be a circular dependency problem we can
   * try to detect that and give a hint by throwing a exception
   */


  //build the dependency tree
  //the dependency tree, pointing to indices in the allOpts
  std::map< int, std::set<int> > dependencyTree;
  for ( int i = 0; unsigned(i) < allOpts.size(); ++i ) {
    const CommandOption &opt = allOpts[i];

    std::set<int> myDeps;
    for ( const std::string &str : opt.dependencies ) {
      int idx = -1;
      if ( str.size() == 1 ) {
        auto it = shortOptIndex.find( str.at(0) );
        if ( it != shortOptIndex.end() ) {
          idx = it->second;
        }
      } else {
        auto it = longOptIndex.find( str );
        if ( it != longOptIndex.end() ) {
          idx = it->second;
        }
      }

      if ( idx == -1 ) {
        throw ZyppFlagsException( str::Format("Flag %1% (%2%) depends on unknown flag %3%") % opt.nameStr() % opt.shortNameStr() % str );
      }
      myDeps.insert( idx );
    }
    dependencyTree.insert( std::make_pair( i, myDeps) );
  }

  //setup getopt
  opterr = 0; 			// we report errors on our own

  //work around the getopt quirks
  optind = 0;       // setting optind to zero will reset the argument parser

  //remember all values we want to write
  //the key of the map is the index of options in allOpts
  std::map<int, std::vector< boost::optional<std::string> > > parsedValues;

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
          ZYPP_THROW( UnknownFlagException( std::string( 1, optopt ) ) );
        else
          ZYPP_THROW( UnknownFlagException( std::string( argv[optind - 1] ) ) );
        break;
      }
      case ':': {
        //the current argument requires a option
        ZYPP_THROW( MissingArgumentException( argv[optind - 1] ) );
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
          if ( optarg ) {
            if ( strlen (optarg ))
              arg = std::string(optarg);
            else
              arg = std::string();
          }

          CommandOption &opt = allOpts[index];

          // check if a conflicting option was used before
          std::vector<std::string> conflictingList;
          for ( const auto &flagSet : conflictingFlags ) {
            if ( std::find( flagSet.begin(), flagSet.end(), opt.name ) != flagSet.end() ) {
              std::copy_if( flagSet.begin(), flagSet.end(), std::back_inserter(conflictingList),  [ &opt ]( const std::string &val ) {
                return val != opt.name;
              });
            }
          }

          if ( !conflictingList.empty() ) {
            for ( const std::string conflicting : conflictingList ){
              auto it = longOptIndex.find( conflicting );
              if ( it == longOptIndex.end() ) {
                WAR << "Ignoring unknown option " << conflicting << " specified as conflicting flag for " << opt.name << endl;
              } else {
                if ( parsedValues.find( it->second ) != parsedValues.end() ) {
                  throw ConflictingFlagsException( opt.nameStr(), CommandOption::nameStr(conflicting) );
                }
              }
            }
          }

          //we remember the given value for now, and write it out in the next step when iterating over the dependency tree
          parsedValues[ index ].push_back( arg );
        }
        break;
      }
    }
  }

  std::set<int> flagsSolved;

  auto allSolved = [ &flagsSolved ]( const int &flagIdx ) {
    return ( flagsSolved.find( flagIdx ) != flagsSolved.end() );
  };

  //we need to run util all flags were solved
  while ( flagsSolved.size() < allOpts.size() ) {

    std::vector<int> writeableFlags;

    for ( const auto &node : dependencyTree ) {

      //all values for that flag where written
      if ( flagsSolved.find( node.first ) != flagsSolved.end() )
        continue;

      std::vector<CommandOption *> opts;
      const std::set<int> &nodeDependencies = node.second;

      if ( nodeDependencies.empty() || std::all_of( nodeDependencies.begin(), nodeDependencies.end(), allSolved ) ) {
        writeableFlags.push_back ( node.first );
      }
    }

    if ( writeableFlags.size() == 0 ) {

      //we were unable to resolve one argument in one pass, give up and check for circular deps (recursive search with copied set)
      std::set< std::string > dep;
      std::function<void ( std::set<int>, int )> findAllDeps = [ &allOpts, &dependencyTree, &findAllDeps] ( std::set < int > foundSoFar, int nextDep ) -> void {
        for ( int i : dependencyTree.at( nextDep ) ) {

          //explicitely do a copy, so we only have one path in the set always
          std::set<int> mySet = foundSoFar;

          if ( ! mySet.insert( i ).second ) {
            //found a circular dependency

            std::string circDep;
            auto appendDepToString = [ &circDep, &allOpts ] ( int depIndex ) {
              auto &opt = allOpts.at( depIndex );
              if ( circDep.size() )
                circDep += "->";
              circDep += opt.nameStr() + "(" + opt.shortNameStr() + ")";
            };

            for ( int dep : foundSoFar )
              appendDepToString(dep);
            appendDepToString(i);

            ZYPP_THROW( ZyppFlagsException ( str::Format("Found a circular dependency: %1%") % circDep ) );
          } else {
            findAllDeps( mySet, i );
          }
        }
      };

      for ( const auto &currDep : dependencyTree )
        findAllDeps ( { currDep.first }, currDep.first );

      break;
    }

    //we now have all writeables for this pass through,
    //lets sort them by priority and finally write them
    std::sort( writeableFlags.begin(), writeableFlags.end(), [ &allOpts ]( int flagA, int flagB  ) {
      return allOpts.at(flagA).priority > allOpts.at(flagB).priority;
    });

    //finally we can write the values
    for ( int flag : writeableFlags ) {
      if ( parsedValues.find( flag ) == parsedValues.end() ) {
        //trigger the "neverUsed" callback if registered
        allOpts[flag].value.neverUsed();
      } else {
        for ( const auto & val : parsedValues.at( flag ) ) {
          allOpts[flag].value.set( allOpts[flag], val );
        }
      }
      flagsSolved.insert( flag );
      parsedValues.erase( flag );
    }
  }

  if ( parsedValues.size() )
    ZYPP_THROW( ZyppFlagsException ("Not all parsed values were handled") );

  return optind;
}

void renderHelp( const std::vector<CommandGroup> &options )
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

      std::cout << "\t" << opt.optionHelp();
      std::cout << std::endl;

    }
    std::cout<<std::endl;
  }
}

CommandOption::CommandOption( std::string &&name_r, char shortName_r, int flags_r, Value &&value_r, std::string &&help_r )
  : name ( std::move(name_r) ),
    shortName ( shortName_r ),
    flags ( flags_r ),
    value ( std::move(value_r) ),
help ( std::move(help_r) )
{ }

std::string CommandOption::optionHelp() const
{
  std::string optHelpTxt = help;
  auto defVal = value.defaultValue();
  if ( defVal && defVal->size() )
    optHelpTxt.append(" ").append(str::Format(("Default: %1%")) %*defVal );
  return optHelpTxt;
}

std::string CommandOption::flagDesc( bool shortOptFirst ) const
{
  std::string optTxt;

  if ( shortName ) {
    if ( shortOptFirst )
      optTxt.append( str::Format("-%1%, --%2%") % shortName % name);
    else
      optTxt.append( str::Format("--%1%, -%2%") % name % shortName);
  } else {
    optTxt.append("--").append( name );
  }

  std::string argSyntax = value.argHint();
  if ( argSyntax.length() ) {
    if ( flags & ZyppFlags::OptionalArgument )
      optTxt.append("[=");
    else
      optTxt.append(" <");
    optTxt.append(argSyntax);
    if ( flags & ZyppFlags::OptionalArgument )
      optTxt.append("]");
    else
      optTxt.append(">");
  }
  return optTxt;
}


std::string CommandOption::nameStr( const std::string & name_r )
{ return( name_r.empty() ? name_r : "--"+name_r ); }

std::string CommandOption::shortNameStr( char shortName_r )
{
  static char buf[] { "-n" };
  if ( shortName_r )
  {
    buf[1] = shortName_r;
    return buf;
  }
  return "";
}

CommandGroup::CommandGroup( std::string &&name_r, std::vector<CommandOption> &&options_r, ConflictingFlagsList &&conflictingOptions_r )
  : name ( std::move(name_r) ),
    options ( std::move(options_r) ),
    conflictingOptions ( std::move(conflictingOptions_r) )
{ }

CommandGroup::CommandGroup( std::vector<CommandOption> &&options_r, ConflictingFlagsList &&conflictingOptions_r )
  : CommandGroup (  _("Command options:"), std::move(options_r), std::move(conflictingOptions_r) )
{ }

CommandGroup::CommandGroup()
  : CommandGroup ( std::vector<CommandOption>(), ConflictingFlagsList() )
{ }


}}

