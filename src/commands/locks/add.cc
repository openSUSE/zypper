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
#include "repos.h"
#include "utils/messages.h"
#include "commands/conditions.h"

using namespace zypp;

std::list<std::string> AddLocksCmd::command() const
{
  return { "addlock", "al", "lock-add", "la" };
}

std::string AddLocksCmd::summary() const
{
  return _("Add a package lock.");
}

std::string AddLocksCmd::synopsis() const
{
  // translators: command synopsis; do not translate the command 'name (abbreviations)' or '-option' names
  return _("addlock (al) [OPTIONS] <PACKAGENAME> ...");
}

std::string AddLocksCmd::description() const
{
  return _("Add a package lock. Specify packages to lock by exact name or by a glob pattern using '*' and '?' wildcard characters.");
}

LoadSystemFlags AddLocksCmd::needSystemSetup() const
{
  return NO_POOL;
}

std::vector<zypp::ZyppFlags::CommandOption> AddLocksCmd::cmdOptions() const
{
  auto that = const_cast<AddLocksCmd *>(this);
  return {
    { "type", 't', ZyppFlags::RequiredArgument, ZyppFlags::KindSetType ( &that->_kinds ) , str::Format(_("Type of package (%1%).") ) % "package, patch, pattern, product" },
    { "repo", 'r', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable, ZyppFlags::StringVectorType ( &that->_repos, "ALIAS|#|URI" ),  _("Restrict the lock to the specified repository.")},
    { "catalog", 'c', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable | ZyppFlags::Hidden, ZyppFlags::StringVectorType ( &that->_repos, "ALIAS|#|URI"),  "Alias for --repo" }
  };
}

void AddLocksCmd::doReset()
{
  _kinds.clear();
  _repos.clear();
}

int AddLocksCmd::execute(Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r)
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
    for_(it,positionalArgs_r.begin(),positionalArgs_r.end())
    {
      PoolQuery q;
      if ( _kinds.empty() ) // derive it from the name
      {
        sat::Solvable::SplitIdent split( *it );
        q.addAttribute( sat::SolvAttr::name, split.name().asString() );
        q.addKind( split.kind() );
      }
      else
      {
        q.addAttribute(sat::SolvAttr::name, *it);
        for_(itk, _kinds.begin(), _kinds.end()) {
          q.addKind(*itk);
        }
      }
      q.setMatchGlob();
      parsed_opts::const_iterator itr;
      //TODO rug compatibility for more arguments with version restrict
      for_(it_repo, _repos.begin(), _repos.end())
      {
        RepoInfo info;
        if( match_repo( zypp_r, *it_repo, &info))
          q.addRepo(info.alias());
        else //TODO some error handling
          WAR << "unknown repository" << *it_repo << endl;
      }
      q.setCaseSensitive();

      locks.addLock(q);
    }
    locks.save(Pathname::assertprefix
        (zypp_r.globalOpts().root_dir, ZConfig::instance().locksFile()));
    if ( start != Locks::instance().size() )
      zypp_r.out().info(PL_(
        "Specified lock has been successfully added.",
        "Specified locks have been successfully added.",
        Locks::instance().size() - start));
  }
  catch(const Exception & e)
  {
    ZYPP_CAUGHT(e);
    zypp_r.out().error(e, _("Problem adding the package lock:"));
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
