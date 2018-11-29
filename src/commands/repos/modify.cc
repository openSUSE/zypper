/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "modify.h"
#include "repos.h"

#include "commands/conditions.h"
#include "commands/commandhelpformatter.h"

#include "utils/messages.h"
#include "utils/flags/flagtypes.h"

using namespace zypp;


ModifyRepoCmd::ModifyRepoCmd(const std::vector<std::string> &commandAliases_r) :
  ZypperBaseCommand (
    commandAliases_r,
    {
      // translators: command synopsis; do not translate lowercase words
      _("modifyrepo (mr) <OPTIONS> <ALIAS|#|URI>"),
      // translators: command synopsis; do not translate lowercase words
      str::Format( _("modifyrepo (mr) <OPTIONS> <%1%>") ) % "--all|--remote|--local|--medium-type"
    },
    // translators: command summary
    _("Modify specified repository."),
    // translators: command description
    str::Format( _("Modify properties of repositories specified by alias, number, or URI, or by the '%1%' aggregate options.") ) % "--all, --remote, --local, --medium-type",
    ResetRepoManager
  )
{ }

std::vector<BaseCommandConditionPtr> ModifyRepoCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}

zypp::ZyppFlags::CommandGroup ModifyRepoCmd::cmdOptions() const
{
  auto that = const_cast<ModifyRepoCmd *>(this);
  return {{
      //some legacy options
      {"legacy-refresh", 'r', ZyppFlags::NoArgument | ZyppFlags::Hidden, ZyppFlags::TriBoolType( that->_commonProps._enableAutoRefresh, ZyppFlags::StoreTrue ), "" },
      {"legacy-no-refresh", 'R', ZyppFlags::NoArgument | ZyppFlags::Hidden, ZyppFlags::TriBoolType( that->_commonProps._enableAutoRefresh, ZyppFlags::StoreFalse), "" }
    }};
}

void ModifyRepoCmd::doReset()
{ }

int ModifyRepoCmd::execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  bool aggregate = _selections._all || _selections._local || _selections._remote || _selections._mediumTypes.size();

  if ( positionalArgs_r.size() < 1 && !aggregate )
  {
    report_alias_or_aggregate_required (zypper.out(), help() );
    ERR << "No alias argument given." << endl;
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  // too many arguments
  if ( positionalArgs_r.size() && aggregate )
  {
    report_too_many_arguments( help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  if ( aggregate )
  {
    modify_repos_by_option( zypper, _selections, _commonProps, _repoProps );
  }
  else
  {
    for_( arg,positionalArgs_r.begin(),positionalArgs_r.end() )
    {
      RepoInfo r;
      if ( match_repo(zypper,*arg,&r) )
      {
        modify_repo( zypper, r.alias(), _commonProps, _repoProps );
      }
      else
      {
       zypper.out().error( str::Format(_("Repository %s not found.")) % *arg );
        ERR << "Repo " << *arg << " not found" << endl;
        return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
      }
    }
  }

  return ZYPPER_EXIT_OK;
}


std::string ModifyRepoCmd::help()
{
  CommandHelpFormater formatter;

  formatter <<  ZypperBaseCommand::help();

  formatter
      .legacyOptionSection()
      .legacyOption( "-r", "-f" )
      .legacyOption( "-R", "-F" );

  return formatter;
}
