/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "remove.h"
#include "repos.h"

#include "commands/conditions.h"
#include "utils/flags/flagtypes.h"
#include "utils/messages.h"
#include "utils/misc.h"
#include "Zypper.h"

using namespace zypp;

RemoveRepoCmd::RemoveRepoCmd( const std::vector<std::string> &commandAliases_r ) :
  ZypperBaseCommand (
    commandAliases_r,
    _("removerepo (rr) [OPTIONS] <ALIAS|#|URI>"),
    _("Remove specified repository."),
    _("Remove repository specified by alias, number or URI."),
    ResetRepoManager
    )
{

}

std::vector<BaseCommandConditionPtr> RemoveRepoCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}

zypp::ZyppFlags::CommandGroup RemoveRepoCmd::cmdOptions() const
{
  //@TODO merge into a OptionSet to share with RemoveServiceCommand ?
  auto that = const_cast<RemoveRepoCmd *>(this);
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

void RemoveRepoCmd::doReset()
{
  _looseAuth = false;
  _looseQuery = false;
}

int RemoveRepoCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  bool aggregate = _selections._all || _selections._local || _selections._remote || _selections._mediumTypes.size();

  if ( positionalArgs_r.size() < 1 && !aggregate )
  {
    report_alias_or_aggregate_required ( zypper.out(), help() );
    ERR << "No alias argument given." << endl;
    return( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  // too many arguments
  if ( positionalArgs_r.size() && aggregate )
  {
    report_too_many_arguments( zypper.out(), help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  if ( aggregate )
  {
    remove_repos_by_option( zypper, _selections );
  }
  else
  {
    // must store repository before remove to ensure correct match number
    std::set<RepoInfo,RepoInfoAliasComparator> repo_to_remove;
    for_(it, positionalArgs_r.begin(), positionalArgs_r.end())
    {
      RepoInfo repo;
      if ( match_repo( zypper ,*it,&repo, _looseQuery, _looseAuth ) )
      {
        repo_to_remove.insert(repo);
      }
      else
      {
        MIL << "Repository not found by given alias, number or URI." << endl;
        // translators: %s is the supplied command line argument which
        // for which no repository counterpart was found
        zypper.out().error( str::Format(_("Repository '%s' not found by alias, number or URI.")) % *it );
      }
    }

    for_(it, repo_to_remove.begin(), repo_to_remove.end())
      remove_repo( zypper, *it );
  }

  return ZYPPER_EXIT_OK;
}

