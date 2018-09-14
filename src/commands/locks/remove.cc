/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "remove.h"

#include <boost/lexical_cast.hpp>

#include <zypp/base/String.h>
#include <zypp/Locks.h>

#include "main.h"
#include "utils/messages.h"
#include "repos.h"
#include "Zypper.h"
#include "utils/flags/flagtypes.h"
#include "commands/conditions.h"

using namespace zypp;

RemoveLocksCmd::RemoveLocksCmd()
  : ZypperBaseCommand(
      { "removelock"	, "rl" , "lock-delete" , "ld" },
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

std::vector<zypp::ZyppFlags::CommandOption> RemoveLocksCmd::cmdOptions() const
{
  auto that = const_cast<RemoveLocksCmd *>(this);
  return {
    { "type", 't', ZyppFlags::RequiredArgument, ZyppFlags::KindSetType ( &that->_kinds ) , str::Format(_("Type of package (%1%).") ) % "package, patch, pattern, product" },
    { "repo", 'r', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable, ZyppFlags::StringVectorType ( &that->_repos, "ALIAS|#|URI" ),  _("Remove only locks with specified repository.") },
    { "catalog", 'c', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable | ZyppFlags::Hidden, ZyppFlags::StringVectorType ( &that->_repos, "ALIAS|#|URI"),  "Alias for --repo" }
  };
}

void RemoveLocksCmd::doReset()
{
  _kinds.clear();
  _repos.clear();
}

int RemoveLocksCmd::execute(Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r)
{
  // too few arguments
  if ( positionalArgs_r.empty() )
  {
    report_required_arg_missing( zypp_r.out(), help() );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }

  try
  {
    Locks & locks = Locks::instance();
    locks.read(Pathname::assertprefix
        (zypp_r.globalOpts().root_dir, ZConfig::instance().locksFile()));
    Locks::size_type start = locks.size();
    for_( args_it, positionalArgs_r.begin(), positionalArgs_r.end() )
    {
      Locks::const_iterator it = locks.begin();
      Locks::LockList::size_type i = 0;
      safe_lexical_cast(*args_it, i);
      if (i > 0 && i <= locks.size())
      {
        advance(it, i-1);
        locks.removeLock(*it);

        zypp_r.out().info(_("Specified lock has been successfully removed."));
      }
      else //package name
      {
        //TODO fill query in one method to have consistent add/remove
        //TODO what to do with repo and _kinds?
        PoolQuery q;
	if ( _kinds.empty() ) // derive it from the name
	{
	  // derive kind from the name: (rl should also support -t)
	  sat::Solvable::SplitIdent split( *args_it );
	  q.addAttribute( sat::SolvAttr::name, split.name().asString() );
	  q.addKind( split.kind() );
	}
	else
	{
	  q.addAttribute(sat::SolvAttr::name, *args_it);
	  for_(itk, _kinds.begin(), _kinds.end()) {
	    q.addKind(*itk);
	  }
	}
	q.setMatchGlob();
        parsed_opts::const_iterator itr;
        for_(it_repo, _repos.begin(), _repos.end())
        {
          RepoInfo info;
          if( match_repo( zypp_r, *it_repo, &info))
            q.addRepo(info.alias());
          else //TODO some error handling
            WAR << "unknown repository" << *it_repo << endl;
        }
        q.setCaseSensitive();

        locks.removeLock(q);
      }
    }

    locks.save(Pathname::assertprefix
        (zypp_r.globalOpts().root_dir, ZConfig::instance().locksFile()));

    // nothing removed
    if (start == locks.size())
      zypp_r.out().info(_("No lock has been removed."));
    //removed something
    else
      zypp_r.out().info(str::form(PL_(
        "%zu lock has been successfully removed.",
        "%zu locks have been successfully removed.",
        start - locks.size()), start - locks.size()));
  }
  catch(const Exception & e)
  {
    ZYPP_CAUGHT(e);
    zypp_r.out().error(e, _("Problem removing the package lock:"));
    return ZYPPER_EXIT_ERR_ZYPP;
  }
  return ZYPPER_EXIT_OK;
}
