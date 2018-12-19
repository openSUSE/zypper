/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "remove.h"
#include "repos.h"

#include "commands/conditions.h"
#include "commands/services/common.h"

#include "utils/misc.h"
#include "utils/messages.h"
#include "utils/flags/flagtypes.h"

RemoveServiceCmd::RemoveServiceCmd(std::vector<std::string> &&commandAliases_r)
  : ZypperBaseCommand (
    std::move( commandAliases_r ),
    _("removeservice (rs) [OPTIONS] <ALIAS|#|URI>"),
    _("Remove specified service."),
    _("Remove specified repository index service from the system."),
    ResetRepoManager
  )
{ }

std::vector<BaseCommandConditionPtr> RemoveServiceCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}

zypp::ZyppFlags::CommandGroup RemoveServiceCmd::cmdOptions() const
{
  auto that = const_cast<RemoveServiceCmd *>(this);
  return {{{
        { "loose-auth", '\0', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_looseAuth, ZyppFlags::StoreTrue),
              // translators: --loose-auth
              _("Ignore user authentication data in the URI.")
        },{ "loose-query", '\0', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_looseQuery, ZyppFlags::StoreTrue),
              // translators: --loose-query
               _("Ignore query string in the URI.")
        }
  }}};
}

void RemoveServiceCmd::doReset()
{
  _looseAuth  = false;
  _looseQuery = false;
}

int RemoveServiceCmd::execute(Zypper &zypp, const std::vector<std::string> &positionalArgs)
{
  if ( positionalArgs.size() < 1)
  {
    ERR << "Required argument missing." << endl;
    report_required_arg_missing( zypp.out(), help() );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }

  std::set<repo::RepoInfoBase_Ptr, ServiceAliasComparator> to_remove;
  for_(it, positionalArgs.begin(), positionalArgs.end())
  {
    repo::RepoInfoBase_Ptr s;
    if (match_service( zypp, *it, s, _looseAuth, _looseQuery))
    {
      to_remove.insert(s);
    }
    else
    {
      MIL << "Service not found by given alias, number or URI." << endl;
      // translators: %s is the supplied command line argument which
      // for which no service counterpart was found
      zypp.out().error( str::Format(_("Service '%s' not found by alias, number or URI.")) % *it );
    }
  }

  for_(it, to_remove.begin(), to_remove.end())
  {
    RepoInfo_Ptr repo_ptr = dynamic_pointer_cast<RepoInfo>(*it);
    if (repo_ptr)
      remove_repo(zypp, *repo_ptr);
    else
      remove_service(zypp, *dynamic_pointer_cast<ServiceInfo>(*it));
  }
  return ZYPPER_EXIT_OK;
}
