/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "remove.h"

#include <zypp/base/String.h>
#include <zypp/Locks.h>

#include "main.h"
#include "utils/messages.h"
#include "Zypper.h"
#include "utils/flags/flagtypes.h"
#include "commands/conditions.h"
#include "commands/commonflags.h"
#include "commands/locks/common.h"

using namespace zypp;

RemoveLocksCmd::RemoveLocksCmd(std::vector<std::string> &&commandAliases_r)
  : ZypperBaseCommand(
      std::move( commandAliases_r ),
      _("removelock (rl) [OPTIONS] <LOCK-NUMBER|PACKAGENAME> ..."),
      _("Remove a package lock."),
      // translators: command description; %1% is acoomand like 'zypper locks'
      str::Format(_("Remove a package lock. Specify the lock to remove by its number obtained with '%1%' or by package name.") ) % "zypper locks",
      DisableAll
  )
{ }

std::vector<BaseCommandConditionPtr> RemoveLocksCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}

ZyppFlags::CommandGroup RemoveLocksCmd::cmdOptions() const
{
  auto that = const_cast<RemoveLocksCmd *>(this);
  return {{
    CommonFlags::resKindSetFlag( that->_kinds ),
    { "repo", 'r', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable, ZyppFlags::StringVectorType ( &that->_repos, "ALIAS|#|URI" ),  _("Remove only locks with specified repository.") },
    { "catalog", 'c', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable | ZyppFlags::Hidden, ZyppFlags::StringVectorType ( &that->_repos, "ALIAS|#|URI"),  "Alias for --repo" }
  }};
}

void RemoveLocksCmd::doReset()
{
  _kinds.clear();
  _repos.clear();
}

int RemoveLocksCmd::execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r)
{
  // too few arguments
  if ( positionalArgs_r.empty() )
  {
    report_required_arg_missing( zypper.out(), help() );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }

  try
  {
    Locks & locks = Locks::instance();
    locks.read(Pathname::assertprefix
        (zypper.config().root_dir, ZConfig::instance().locksFile()));
    Locks::size_type start = locks.size();
    for_( args_it, positionalArgs_r.begin(), positionalArgs_r.end() )
    {
      Locks::const_iterator it = locks.begin();
      Locks::LockList::size_type i = 0;
      str::strtonum(*args_it, i);
      if (i > 0 && i <= locks.size())
      {
        advance(it, i-1);
        locks.removeLock(*it);

        zypper.out().info(_("Specified lock has been successfully removed."));
      }
      else //package name
      {
        locks.removeLock( locks::arg2query( zypper, *args_it, _kinds, _repos, "" ) );
      }
    }

    locks.save(Pathname::assertprefix
        (zypper.config().root_dir, ZConfig::instance().locksFile()));

    // nothing removed
    if (start == locks.size())
      zypper.out().info(_("No lock has been removed."));
    //removed something
    else
      zypper.out().info(str::form(PL_(
        "%zu lock has been successfully removed.",
        "%zu locks have been successfully removed.",
        start - locks.size()), start - locks.size()));
  }
  catch(const Exception & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e, _("Problem removing the package lock:"));
    return ZYPPER_EXIT_ERR_ZYPP;
  }
  return ZYPPER_EXIT_OK;
}
