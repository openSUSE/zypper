#include "clean.h"

#include <zypp/Locks.h>

#include "main.h"
#include "Zypper.h"
#include "commands/conditions.h"
#include "utils/flags/flagtypes.h"
#include "utils/messages.h"

using namespace zypp;

std::list<std::string> CleanLocksCmd::command() const
{
  return { "cleanlocks" , "cl" , "lock-clean" };
}

std::string CleanLocksCmd::summary() const
{
  return description();
}

std::string CleanLocksCmd::synopsis() const
{
  // translators: command synopsis; do not translate the command 'name (abbreviations)' or '-option' names
  return _("cleanlocks (cl)");
}

std::string CleanLocksCmd::description() const
{
  // translators: command description
  return _("Remove useless locks.");
}

LoadSystemFlags CleanLocksCmd::needSystemSetup() const
{
  //load all
  return LoadSystemFlags();
}

std::vector<BaseCommandConditionPtr> CleanLocksCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}

std::vector<zypp::ZyppFlags::CommandOption> CleanLocksCmd::cmdOptions() const
{
  auto that = const_cast<CleanLocksCmd *>(this);
  return {
    {"only-duplicates", 'd', ZyppFlags::NoArgument, ZyppFlags::BoolType(&that->_onlyDuplicates, ZyppFlags::StoreTrue, _onlyDuplicates),
          // translators: -d, --only-duplicates
          _("Clean only duplicate locks.")},
    {"only-empty", 'e', ZyppFlags::NoArgument, ZyppFlags::BoolType(&that->_onlyEmpty, ZyppFlags::StoreTrue, _onlyEmpty),
          // translators: -e, --only-empty
          _("Clean only locks which doesn't lock anything.") },
  };
}

void CleanLocksCmd::doReset()
{
  _onlyDuplicates = false;
  _onlyEmpty = false;
}

int CleanLocksCmd::execute(Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r)
{
  if ( !positionalArgs_r.empty() ) {
    report_too_many_arguments( help() );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }

  Locks::instance().read();
  Locks::size_type start = Locks::instance().size();
  if ( !_onlyDuplicates )
    Locks::instance().removeEmpty();
  if ( !_onlyEmpty )
    Locks::instance().removeDuplicates();

  Locks::instance().save();

  Locks::size_type diff = start - Locks::instance().size();
  zypp_r.out().info( str::form( PL_("Removed %lu lock.","Removed %lu locks.", diff), (long unsigned)diff ) );

  return ZYPPER_EXIT_OK;
}
