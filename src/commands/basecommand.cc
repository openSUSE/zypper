#include "basecommand.h"

#include <boost/optional.hpp>
#include <zypp/base/LogControl.h>
#include "utils/flags/flagtypes.h"
#include "utils/messages.h"
#include "commandhelpformatter.h"
#include "solve-commit.h"
#include "global-settings.h"

#include "src/repos.h"

using namespace zypp;

extern ZYpp::Ptr God;

std::ostream & operator<<( std::ostream & str, const SetupSystemFlags & obj )
{
#define OUTS(F) { F, #F }
  static const std::initializer_list<std::pair<SetupSystemFlags,std::string> > strmap {
    OUTS( ResetRepoManager ),
    OUTS( InitTarget ),
    OUTS( InitRepos ),
    OUTS( NoSystemResolvables ),
    OUTS( LoadTargetResolvables ),
    OUTS( LoadRepoResolvables ),
    OUTS( LoadResolvables ),
    OUTS( Resolve ),
  };
#undef OUTS
  return str << zypp::base::stringify( obj, strmap );
}

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

ZypperBaseCommand::ZypperBaseCommand(std::vector<std::string> &&commandAliases_r, std::string &&synopsis_r,
                                     std::string &&summary_r, CommandDescription &&description_r,
                                     SetupSystemFlags systemInitFlags_r)
  : ZypperBaseCommand( std::move(commandAliases_r), std::vector<std::string>{ std::move(synopsis_r) }, std::move(summary_r), std::move(description_r), systemInitFlags_r )
{

}

ZypperBaseCommand::ZypperBaseCommand(std::vector<std::string> &&commandAliases_r, std::vector<std::string> &&synopsis_r,
                                     std::string &&summary_r, CommandDescription &&description_r,
                                     SetupSystemFlags systemInitFlags_r)
  : _commandAliases ( std::move( commandAliases_r ) ),
    _synopsis ( std::move( synopsis_r ) ),
    _summary ( std::move( summary_r ) ),
    _description ( std::move( description_r ) ),
    _systemInitFlags ( systemInitFlags_r )
{

}

ZypperBaseCommand::~ZypperBaseCommand()
{
}

void ZypperBaseCommand::reset()
{
  _helpRequested = false;

  //reset the global settings
  GlobalSettings::reset();

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

int ZypperBaseCommand::parseArguments( Zypper &, const int argc, char * const *argv )
{

  int nextArg = 0;

  if ( _parseArguments )
    nextArg = ZyppFlags::parseCLI( argc, argv, options() );
  else
    nextArg = argc; //we eat all arguments

  MIL << "Done parsing options." << endl;

  if ( _fillRawOptions ) {
    //skip first argument here as well
    for ( int i = 1; i < argc; ++i ) {
      _rawOptions.push_back( argv[ i ] );
    }
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

  DtorReset _tmp( zypper.configNoConst().disable_system_resolvables );
  if ( flags_r.testFlag( NoSystemResolvables ) ) {
    zypper.configNoConst().disable_system_resolvables = true;
  }

  if ( flags_r.testFlag( LoadResolvables ) ) {
    load_resolvables( zypper );
  } else if ( flags_r.testFlag( LoadRepoResolvables ) ) {
    load_repo_resolvables( zypper );
  } else if ( flags_r.testFlag( LoadTargetResolvables ) ) {
    load_target_resolvables( zypper );
  }

  if ( zypper.exitCode() != ZYPPER_EXIT_OK )
    return zypper.exitCode();

#if 0 // since libzypp 17.23.0 the Resolver auto establishes the PPP status.
  if ( flags_r.testFlag ( Resolve ) ) {
    // have REPOS and TARGET
    // compute status of PPP
    MIL << "-------------- Calling SAT Solver to establish the PPP status -------------------" << endl;
    base::LogControl::TmpLineWriter shutUp;	// reduce logging; some day libzypp/libsolv may offer a shotcut to establish
    resolve( zypper );
  }
#endif

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

void ZypperBaseCommand::disableArgumentParser( bool disable )
{
  _parseArguments = !disable;
}

const std::vector<std::string> &ZypperBaseCommand::rawOptions() const
{
  return _rawOptions;
}

std::vector<std::string> ZypperBaseCommand::positionalArguments() const
{
  return _positionalArguments;
}

void ZypperBaseCommand::setPositionalArguments(const std::vector<std::string> &positionalArguments)
{
  _positionalArguments = positionalArguments;
}

int ZypperBaseCommand::earlyPositionalArgsCheck( Zypper &zypper, const std::vector<std::string> &positionalArgs )
{ return ZYPPER_EXIT_OK; }

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

    if ( int code = earlyPositionalArgsCheck( zypper, _positionalArguments ); code != ZYPPER_EXIT_OK )
      return code;

    if ( int code = systemSetup( zypper ); code != ZYPPER_EXIT_OK )
      return code;

    return execute( zypper, _positionalArguments );
  }

  // ZypperBaseCommand's should actually handle all exceptions themselfes, however
  // for compatibility reasons we better catch those here again
  //
  // TODO Someday redesign the Exceptions flow.
  catch ( const Out::Error & error_r )
  {
    error_r.report( zypper );
    return error_r._exitcode;
  }
  catch ( const AbortRequestException & ex )
  {
    ZYPP_CAUGHT( ex );
    zypper.out().error( ex.asUserString() );
  }
  catch ( const ExitRequestException & ex )
  {
    ZYPP_CAUGHT( ex );
    WAR << "Caught exit request: exitCode " << zypper.exitCode() << endl;
  }
  catch ( const Exception & ex )
  {
    ZYPP_CAUGHT( ex );
    {
      SCOPED_VERBOSITY( zypper.out(), Out::DEBUG );
      zypper.out().error( ex, _("Unexpected exception.") );
    }
    report_a_bug( zypper.out() );
    if ( ! zypper.exitCode() )
      zypper.setExitCode( ZYPPER_EXIT_ERR_BUG );
  }

  //if we reach this place we catched a execption the command should've handled,
  //lets hope it did actually set the exit code
  return zypper.exitCode();
}

std::string ZypperBaseCommand::help()
{
  CommandHelpFormater help;
  for ( const std::string &syn : synopsis() )
    help.synopsis(syn);
  help.description(description());

  auto renderOption = [&help]( const ZyppFlags::CommandOption &opt ) {
    help.option( opt.flagDesc(), opt.optionHelp() );
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
        //even though the option might be hidden, we still do not want to show the
        //"no options" available message anymore. Its misleading.
        hadOptions = true;
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

