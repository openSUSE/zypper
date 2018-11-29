/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "rename.h"
#include "repos.h"

#include "commands/conditions.h"
#include "utils/messages.h"

#include "Zypper.h"

using namespace zypp;

namespace {

/**
 * Rename repository specified by \a alias to \a newalias.
 */
int rename_repo( Zypper & zypper, const std::string & alias, const std::string & newalias )
{
  RepoManager & manager( zypper.repoManager() );

  try
  {
    RepoInfo repo( manager.getRepositoryInfo( alias ) );

    if ( !repo.service().empty() )
    {
      zypper.out().error(str::form(
          _("Cannot change alias of '%s' repository. The repository"
            " belongs to service '%s' which is responsible for setting its alias."),
          alias.c_str(), repo.service().c_str()));
      return ZYPPER_EXIT_ERR_ZYPP;
    }

    repo.setAlias( newalias );
    manager.modifyRepository( alias, repo );

    MIL << "Repository '" << alias << "' renamed to '" << repo.alias() << "'" << endl;
    zypper.out().info( str::Format(_("Repository '%s' renamed to '%s'.")) % alias % repo.alias() );
  }
  catch ( const repo::RepoAlreadyExistsException & ex )
  {
    zypper.out().error( str::Format(_("Repository named '%s' already exists. Please use another alias.")) % newalias );
  }
  catch ( const Exception & ex )
  {
    ERR << "Error while modifying the repository " << ex.asUserString() << endl;
    zypper.out().error( ex, _("Error while modifying the repository:"),
			str::Format(_("Leaving repository '%s' unchanged.")) % alias );
  }
  return ZYPPER_EXIT_OK;
}

}

RenameRepoCmd::RenameRepoCmd(const std::vector<std::string> &commandAliases_r) :
  ZypperBaseCommand (
    commandAliases_r,
    // translators: command synopsis; do not translate lowercase words
    _("renamerepo (nr) [OPTIONS] <ALIAS|#|URI> <NEW-ALIAS>"),
    _("Rename specified repository."),
    // translators: command description
    _("Assign new alias to the repository specified by alias, number or URI."),
    ResetRepoManager
    )
{ }

std::vector<BaseCommandConditionPtr> RenameRepoCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}

zypp::ZyppFlags::CommandGroup RenameRepoCmd::cmdOptions() const
{
  return {};
}

void RenameRepoCmd::doReset()
{ }

int RenameRepoCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{

  if ( positionalArgs_r.size() < 2 )
  {
    zypper.out().error(_("Too few arguments. At least URI and alias are required.") );
    ERR << "Too few arguments. At least URI and alias are required." << endl;
    zypper.out().info( help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }
  // too many arguments
  else if ( positionalArgs_r.size() > 2 )
  {
    report_too_many_arguments( help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  try
  {
    RepoInfo repo;
    if ( match_repo( zypper,positionalArgs_r[0], &repo  ))
    {
      int code = rename_repo( zypper, repo.alias(), positionalArgs_r[1] );
      return code;
    }
    else
    {
       zypper.out().error( str::Format(_("Repository '%s' not found.")) % positionalArgs_r[0] );
       ERR << "Repo " << positionalArgs_r[0] << " not found" << endl;
    }
  }
  catch ( const Exception & excpt_r )
  {
    zypper.out().error( excpt_r.asUserString() );
    return ( ZYPPER_EXIT_ERR_ZYPP );
  }

  return ZYPPER_EXIT_OK;
}
