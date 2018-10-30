#include "basecommand.h"

#include <boost/optional.hpp>
#include "utils/flags/flagtypes.h"
#include "commandhelpformatter.h"
#include "solve-commit.h"

#include "src/repos.h"

using namespace zypp;

extern ZYpp::Ptr God;

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

ZypperBaseCommand::ZypperBaseCommand(const std::vector<std::string> &commandAliases_r, const std::string &synopsis_r,
                                     const std::string &summary_r, const std::string &description_r,
                                     SetupSystemFlags systemInitFlags_r)
  : ZypperBaseCommand( commandAliases_r, std::vector<std::string>{synopsis_r}, summary_r, description_r, systemInitFlags_r )
{

}

ZypperBaseCommand::ZypperBaseCommand(const std::vector<std::string> &commandAliases_r, const std::vector<std::string> &synopsis_r,
                                     const std::string &summary_r, const std::string &description_r,
                                     SetupSystemFlags systemInitFlags_r)
  : _commandAliases ( commandAliases_r ),
    _synopsis ( synopsis_r ),
    _summary ( summary_r ),
    _description ( description_r ),
    _systemInitFlags ( systemInitFlags_r )
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

  _positionalArguments.clear();
  _rawOptions.clear();

  //call the commands own implementation
  doReset();
}

std::vector<std::string> ZypperBaseCommand::command() const
{
  return _commandAliases;
}

std::string ZypperBaseCommand::summary() const
{
  return _summary;
}

std::vector<std::string> ZypperBaseCommand::synopsis() const
{
  return _synopsis;
}

std::string ZypperBaseCommand::description() const
{
  return _description;
}

int ZypperBaseCommand::parseArguments(Zypper &zypper, const int firstOpt )
{
  const int argc    = zypper.argc();
  char * const *argv = zypper.argv();
  int nextArg = ZyppFlags::parseCLI( argc, argv, options(), firstOpt );

  MIL << "Parsed new style arguments" << endl;

  if ( _fillRawOptions ) {
    std::ostringstream s;
    s << _("Option program arguments: ");
    for ( int i = firstOpt; i < nextArg; i++ ) {
      std::string argument = argv[ i ];
      s << "'" << argument << "' ";
      _rawOptions.push_back( argument );
    }
    zypper.out().info( s.str(), Out::HIGH );
  }

  if ( nextArg < argc )
  {
    std::ostringstream s;
    s << _("Non-option program arguments: ");
    while ( nextArg < argc )
    {
      std::string argument = argv[nextArg++];
      s << "'" << argument << "' ";
      _positionalArguments.push_back( argument );
    }
    zypper.out().info( s.str(), Out::HIGH );
  }

  return nextArg;
}

bool ZypperBaseCommand::helpRequested() const
{
  return _helpRequested;
}

SetupSystemFlags ZypperBaseCommand::setupSystemFlags() const
{
  return _systemInitFlags;
}

ZYpp::Ptr ZypperBaseCommand::zyppApi() const
{
  return God;
}

std::vector<BaseCommandConditionPtr> ZypperBaseCommand::conditions() const
{
  return std::vector<BaseCommandConditionPtr>();
}

int ZypperBaseCommand::systemSetup( Zypper &zypper )
{
  return defaultSystemSetup ( zypper, _systemInitFlags );
}

int ZypperBaseCommand::defaultSystemSetup( Zypper &zypper, SetupSystemFlags flags_r )
{
  DBG << "FLAGS:" << flags_r << endl;

  if ( flags_r.testFlag( ResetRepoManager ) )
    zypper.initRepoManager();

  if ( flags_r.testFlag( InitTarget ) ) {
    init_target( zypper );
    if ( zypper.exitCode() != ZYPPER_EXIT_OK )
      return zypper.exitCode();
  }

  if ( flags_r.testFlag( InitRepos ) ) {
    init_repos( zypper );
    if ( zypper.exitCode() != ZYPPER_EXIT_OK )
      return zypper.exitCode();
  }

  DtorReset _tmp( zypper.globalOptsNoConst().disable_system_resolvables );
  if ( flags_r.testFlag( LoadResolvables ) ) {
    if ( flags_r.testFlag( NoSystemResolvables ) ) {
      zypper.globalOptsNoConst().disable_system_resolvables = true;
    }

    load_resolvables( zypper );
    if ( zypper.exitCode() != ZYPPER_EXIT_OK )
      return zypper.exitCode();
  }

  if ( flags_r.testFlag ( Resolve ) ) {
    // have REPOS and TARGET
    // compute status of PPP
    resolve( zypper );
  }

  return zypper.exitCode();
}

bool ZypperBaseCommand::fillRawOptions() const
{
  return _fillRawOptions;
}

void ZypperBaseCommand::setFillRawOptions(bool fillRawOptions)
{
  _fillRawOptions = fillRawOptions;
}

std::vector<std::string> ZypperBaseCommand::rawOptions() const
{
  return _rawOptions;
}

std::vector<std::string> ZypperBaseCommand::positionalArguments() const
{
  return _positionalArguments;
}

int ZypperBaseCommand::run( Zypper &zypper )
{
  MIL << "run: " << command().front() << endl;
  try
  {
    for ( const BaseCommandConditionPtr &cond : conditions() ) {
      std::string error;
      int code = cond->check( error );
      if ( code != 0 ) {
        zypper.out().error( error );
        return code;
      }
    }

    int code = systemSetup( zypper );
    if ( code != ZYPPER_EXIT_OK )
      return code;

    return execute( zypper, _positionalArguments );
  }
  catch ( const Out::Error & error_r )
  {
    error_r.report( zypper );
    return error_r._exitcode;
  }

  //@TODO report bug
  return 0;
}

std::string ZypperBaseCommand::help()
{
  CommandHelpFormater help;
  for ( const std::string &syn : synopsis() )
    help.synopsis(syn);
  help.description(description());

  auto renderOption = [&help]( const ZyppFlags::CommandOption &opt ) {
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
  };

  //all the options we have
  std::vector<ZyppFlags::CommandGroup> opts = options();

  //collect all deprecated options
  std::vector<const ZyppFlags::CommandOption*> legacyOptions;

  bool hadOptions = false;
  for ( const ZyppFlags::CommandGroup &grp : opts ) {
    if ( grp.options.size() ) {
      bool wroteSectionHdr = false;
      for ( const ZyppFlags::CommandOption &opt : grp.options ) {
        if ( opt.flags & ZyppFlags::Hidden )
          continue;
        if ( opt.flags & ZyppFlags::Deprecated ) {
          legacyOptions.push_back( &opt );
          continue;
        }
        //write the section header only if we actuall have entries
        if ( !wroteSectionHdr ) {
          wroteSectionHdr = true;
          help.optionSection( grp.name );
        }
        hadOptions = true;
        renderOption(opt);
      }
    }
  }

  if ( legacyOptions.size() ) {
    help.legacyOptionSection();
    for ( const ZyppFlags::CommandOption *legacyOption : legacyOptions ) {
      renderOption(*legacyOption);
    }
  }

  if ( !hadOptions ) {
    help.noOptionSection();
  }

  return help;
}

std::vector<ZyppFlags::CommandGroup> ZypperBaseCommand::options()
{
  //first get the commands own options
  std::vector<ZyppFlags::CommandGroup> allOpts;

  //merges a group into
  auto mergeGroup = [ &allOpts ] ( ZyppFlags::CommandGroup &&grp ) {
    bool foundCmdGroup = false;
    for ( ZyppFlags::CommandGroup &cmdGroup : allOpts ) {
      if ( grp.name == cmdGroup.name ) {
        foundCmdGroup = true;
        std::move( grp.options.begin(), grp.options.end(), std::back_inserter(cmdGroup.options) );
        std::move( grp.conflictingOptions.begin(), grp.conflictingOptions.end(), std::back_inserter(cmdGroup.conflictingOptions) );
      }
    }
    if ( !foundCmdGroup )
      allOpts.push_back( std::move(grp) );
  };

  //inject a help option at the beginning, we always want help
  //this will also make sure that the default command group is at the beginning
  mergeGroup ( ZyppFlags::CommandGroup {{
    { "help", 'h', ZyppFlags::NoArgument | ZyppFlags::Hidden, ZyppFlags::BoolType( &_helpRequested ), "" }
  }});

  // now add the commands own options
  mergeGroup ( cmdOptions() );

  //now collect all options from registered option sets
  for ( BaseCommandOptionSet *set : _registeredOptionSets ) {

    std::vector<ZyppFlags::CommandGroup> setOpts = set->options();

    //merge the set into the other options
    for ( ZyppFlags::CommandGroup &grp : setOpts ) {
      mergeGroup ( std::move(grp) );
    }
  }
  return allOpts;
}

void ZypperBaseCommand::addOptionSet(BaseCommandOptionSet &set)
{
  _registeredOptionSets.push_back(&set);
}

