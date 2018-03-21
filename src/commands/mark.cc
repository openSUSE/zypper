/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>
#include <zypp/base/LogTools.h>
#include <zypp/sat/Queue.h>
#include <zypp/sat/Pool.h>
#include <zypp/ZYpp.h>

#include "Zypper.h"
#include "repos.h"
#include "mark.h"
#include "utils/misc.h"
#include "utils/messages.h"
#include "PackageArgs.h"
#include "SolverRequester.h"
#include "global-settings.h"
#include "commands/commonflags.h"

namespace  {

enum class Mode {
  INVALID,
  AUTO,
  MANUAL
};

Mode modeFromString( const std::string &str_r )
{
  if ( str_r == "manual" )
    return Mode::MANUAL;
  else if ( str_r == "auto" )
    return Mode::AUTO;
  return Mode::INVALID;
}

std::string modeToString( const Mode mode_r )
{
  switch (mode_r) {
    case Mode::MANUAL:
      return "manual";
    case Mode::AUTO:
      return "auto";
    default:
      return "invalid";
  }
}

}

MarkCmd::MarkCmd( std::vector<std::string> &&commandAliases_r )
  : ZypperBaseCommand (
      std::move( commandAliases_r ),
      // translators: command synopsis; do not translate lowercase words
      _("mark [OPTIONS] auto|manual PACKAGE.."),
      // translators: command summary
      _("Mark or unmark installed packages as having been automatically installed."),
      // translators: command description
      _("Mark or unmark installed packages as having been automatically installed. Automatically installed packages may later be removed, if no more manually installed package depends on them."),
      InitTarget | LoadResolvables
    )
{
  init();
}

void MarkCmd::init()
{
  _kind = ResKind::package;
  _selectByCap = false;
  _selectByName = false;
}

ZyppFlags::CommandGroup MarkCmd::cmdOptions() const
{
  auto &that = *const_cast<MarkCmd *>(this);
  return {{
    {"type", 't', ZyppFlags::RequiredArgument, ZyppFlags::GenericValueType( that._kind, ARG_TYPE ), _("Type of package to consider. The default is to prefer packages.")},
    CommonFlags::selectByPackageNameFlag( that._selectByName ),
    CommonFlags::selectByPackageCapFlag( that._selectByCap )
    },{
      { "capability", "name" }
    }};
}

void MarkCmd::doReset()
{
  init();
}

std::vector<BaseCommandConditionPtr> MarkCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}

int MarkCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs )
{
  if ( positionalArgs.empty() )
  {
    report_required_arg_missing( zypper.out(), help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  //the first argument is the mark mode, cut it out and set in options
  const std::string &cmd = positionalArgs.front();
  Mode mode = modeFromString(cmd);
  if ( mode == Mode::INVALID )
  {
    report_required_arg_missing( zypper.out(), help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  std::vector<std::string> pkgArgs( positionalArgs.begin() + 1 , positionalArgs.end() );

  if ( pkgArgs.empty() ) {
    report_required_arg_missing( zypper.out(), help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  // parse package arguments
  PackageArgs args( pkgArgs, _kind );

  // tell the solver what we want
  SolverRequester::Options sropts;
  sropts.force_by_cap  = _selectByCap;
  sropts.force_by_name = _selectByName;

  bool listDirty = false;
  bool dryRun = DryRunSettings::instance().isEnabled();

  SolverRequester sr( sropts );

  //all currently auto installed packages
  sat::StringQueue ai( sat::Pool::instance().autoInstalled() );

  zypper.out().gap();

  sr.apply( args, [&]( PoolItem item_r ) {
    if ( mode == Mode::MANUAL && ai.contains(item_r.ident().id()) )
    {
      if ( dryRun )
        zypper.out().info( str::Str() << _("Not marking package as manually installed: ")<< item_r.ident() << " (--dry-run)" );
      else
      {
        listDirty = true;
        zypper.out().info( str::Str() << _("Marking package as manually installed: ")<< item_r.ident() );
        ai.remove( item_r.ident().id() );
      }

    }
    else if ( mode == Mode::AUTO && !ai.contains(item_r.ident().id()) ) {
      if ( dryRun )
        zypper.out().info( str::Str() << _("Not marking package as auto installed: ")<< item_r.ident() << " (--dry-run)" );
      else
      {
        listDirty = true;
        zypper.out().info( str::Str() << _("Marking package as auto installed: ")<< item_r.ident() );
        ai.push_back( item_r.ident().id() );
      }
    }
    else {
      zypper.out().info( str::Str() << _("Ignoring already marked package: ")<< item_r.ident() );
    }
    return true;
  });

  sr.printFeedback( zypper.out() );

  if ( !zypper.config().ignore_unknown
       && ( sr.hasFeedback( SolverRequester::Feedback::NOT_FOUND_NAME )
            || sr.hasFeedback( SolverRequester::Feedback::NOT_FOUND_CAP ) ) )
  {
    zypper.setExitCode( ZYPPER_EXIT_INF_CAP_NOT_FOUND );
    ZYPP_THROW( ExitRequestException("name or capability not found") );
  }

  if ( listDirty )
  {
    sat::Pool::instance().setAutoInstalled( ai );
    zyppApi()->target()->updateAutoInstalled();
  }

  return zypper.exitCode();
}
