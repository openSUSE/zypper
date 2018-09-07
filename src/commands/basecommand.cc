#include "basecommand.h"

#include <boost/optional.hpp>
#include "utils/flags/flagtypes.h"
#include "commandhelpformatter.h"

using namespace zypp;

BaseCommandOptionSet::BaseCommandOptionSet()
{
}

BaseCommandOptionSet::BaseCommandOptionSet(ZypperBaseCommand &parent)
{
  parent.addOptionSet(*this);
}

BaseCommandOptionSet::~BaseCommandOptionSet()
{
}

BaseCommandCondition::~BaseCommandCondition()
{

}

ZypperBaseCommand::~ZypperBaseCommand()
{
}

void ZypperBaseCommand::reset()
{
  _helpRequested = false;

  //reset all registered option sets
  for ( BaseCommandOptionSet *set : _registeredOptionSets )
    set->reset();

  //call the commands own implementation
  doReset();
}

bool ZypperBaseCommand::helpRequested() const
{
  return _helpRequested;
}

std::vector<BaseCommandConditionPtr> ZypperBaseCommand::conditions() const
{
  return std::vector<BaseCommandConditionPtr>();
}

LoadSystemFlags ZypperBaseCommand::needSystemSetup() const
{
  //we want all by default
  return LoadSystemFlags();
}

int ZypperBaseCommand::run(Zypper &zypp, const std::vector<std::string> &positionalArgs)
{
  MIL << "run: " << command().front() << endl;
  try
  {
    for ( const BaseCommandConditionPtr cond : conditions() ) {
      std::string error;
      int code = cond->check( error );
      if ( code != 0 ) {
        zypp.out().error( error );
        return code;
      }
    }

    return execute( zypp, positionalArgs );
  }
  catch ( const Out::Error & error_r )
  {
    error_r.report( zypp );
    return error_r._exitcode;
  }

  //@TODO report bug
  return 0;
}

std::string ZypperBaseCommand::help()
{
  CommandHelpFormater help;
  help.synopsis(synopsis())
      .description(description());

  bool hadOptions = false;
  for ( const ZyppFlags::CommandGroup &grp : options() ) {
    if ( grp.options.size() ) {
      hadOptions = true;
      help.optionSection( grp.name );
      for ( const ZyppFlags::CommandOption &opt : grp.options ) {
        if ( opt.flags & ZyppFlags::Hidden )
          continue;

        std::string optTxt;
        if ( opt.shortName )
          optTxt.append( str::Format("-%1%, ") % opt.shortName);
        optTxt.append("--").append(opt.name);

        std::string argSyntax = opt.value.argHint();
        if ( argSyntax.length() ) {
          if ( opt.flags & ZyppFlags::OptionalArgument )
            optTxt.append("[=");
          else
            optTxt.append(" <");
          optTxt.append(argSyntax);
          if ( opt.flags & ZyppFlags::OptionalArgument )
            optTxt.append("]");
          else
            optTxt.append(">");
        }

        std::string optHelpTxt = opt.help;
        auto defVal = opt.value.defaultValue();
        if ( defVal )
          optHelpTxt.append(" ").append(str::Format(("Default: %1%")) %*defVal );
        help.option(optTxt, optHelpTxt);
      }
    }
  }

  return help;
}

std::vector<ZyppFlags::CommandGroup> ZypperBaseCommand::options()
{
  //first get the commands own options
  std::vector<ZyppFlags::CommandGroup> allOpts {
    { _("Command options:"), cmdOptions() }
  };

  ZyppFlags::CommandOption helpOpt{
      "help", 'h', ZyppFlags::NoArgument | ZyppFlags::Hidden, ZyppFlags::BoolType( &_helpRequested ), ""
  };

  //inject a help option at the beginning, we always want help
  allOpts.front().options.insert( allOpts.front().options.end(), std::move(helpOpt));

  //now collect all options from registered option sets
  for ( BaseCommandOptionSet *set : _registeredOptionSets ) {

    std::vector<ZyppFlags::CommandGroup> setOpts = set->options();

    //merge the set into the other options
    for ( ZyppFlags::CommandGroup &grp : setOpts ) {
      bool foundCmdGroup = false;
      for ( ZyppFlags::CommandGroup &cmdGroup : allOpts ) {
        if ( grp.name == cmdGroup.name ) {
          foundCmdGroup = true;
          std::move(grp.options.begin(), grp.options.end(), std::back_inserter(cmdGroup.options));
        }
      }
      if ( !foundCmdGroup )
        allOpts.push_back( grp );
    }
  }

  return allOpts;
}

void ZypperBaseCommand::addOptionSet(BaseCommandOptionSet &set)
{
  _registeredOptionSets.push_back(&set);
}

