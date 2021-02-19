/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "add.h"

#include <iostream>

#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Locks.h>

#include "utils/flags/flagtypes.h"

#include "Zypper.h"
#include "utils/messages.h"
#include "commands/conditions.h"
#include "commands/commonflags.h"
#include "commands/locks/common.h"

using namespace zypp;

AddLocksCmd::AddLocksCmd(std::vector<std::string> &&commandAliases_r ) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate the command 'name (abbreviations)' or '-option' names
    _("addlock (al) [OPTIONS] <LOCKSPEC>..."),
    // translators: command summary
    _("Add a package lock."),
    // translators: command description
    {
      _("Add a package lock."),
      _("LOCKSPEC is formed by '[KIND:]NAME[ OP EDITION]', where NAME may also be a glob pattern using * and ? wildcard characters. Non-package types may to have their KIND: string prepended (e.g. 'patch:foo') or use the commands --type option."),
      _("The basic form will lock all editions of the matching items. You can optionally restrict the lock to match a specific edition or edition range using =, <, <=, >, >= or != followed an EDITION.")
    },
    DisableAll )
{ }

ZyppFlags::CommandGroup AddLocksCmd::cmdOptions() const
{
  auto that = const_cast<AddLocksCmd *>(this);
  return {{
    CommonFlags::resKindSetFlag( that->_kinds ),
    { "repo", 'r', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable, ZyppFlags::StringVectorType ( &that->_repos, "ALIAS|#|URI" ),  _("Restrict the lock to the specified repository.")},
    { "catalog", 'c', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable | ZyppFlags::Hidden, ZyppFlags::StringVectorType ( &that->_repos, "ALIAS|#|URI"),  "Alias for --repo" }
  }};
}

void AddLocksCmd::doReset()
{
  _kinds.clear();
  _repos.clear();
}

int AddLocksCmd::execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r)
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
    for_(it,positionalArgs_r.begin(),positionalArgs_r.end())
    {
      locks.addLock( locks::arg2query( zypper, *it, _kinds, _repos ) );
    }
    locks.save(Pathname::assertprefix
        (zypper.config().root_dir, ZConfig::instance().locksFile()));
    if ( start != Locks::instance().size() )
      zypper.out().info(PL_(
        "Specified lock has been successfully added.",
        "Specified locks have been successfully added.",
        Locks::instance().size() - start));
  }
  catch(const Exception & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e, _("Problem adding the package lock:"));
    return ZYPPER_EXIT_ERR_ZYPP;
  }
  return ZYPPER_EXIT_OK;
}

std::vector<BaseCommandConditionPtr> AddLocksCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}
