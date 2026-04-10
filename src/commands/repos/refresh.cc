/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "refresh.h"
#include "repos.h"
#include "commands/conditions.h"
#include "commands/services/refresh.h"


#include "utils/messages.h"
#include "utils/flags/flagtypes.h"
#include "Zypper.h"

using namespace zypp;

extern ZYpp::Ptr God;

namespace {
  // bsc#1234752: Try to refresh update repos first (to have updated GPG keys on the fly).
  // GA repos usually ship the old, maybe meanwhile expired, GPG key. If such a key was
  // prolonged, the update repo may contain it and zypp updates the trusted key on the fly
  // when refreshing it. This avoids a 'key has expired' warning being issued when refreshing
  // the GA repos
  inline std::list<RepoInfo> inRefreshOrder( std::list<RepoInfo> && list_r )
  {
    list_r.sort( []( const RepoInfo & lhs, const RepoInfo & rhs ) {
      static const std::string update { "update" };
      // 'update' directory within the URS's path
      bool lu = str::containsCI( lhs.url().getPathName(), update );
      bool ru = str::containsCI( rhs.url().getPathName(), update );
      if ( lu != ru )
        return lu;      // update in path wins
      const std::string & la { lhs.alias() };
      const std::string & ra { rhs.alias() };
      if ( lu )
        return la < ra; // both update => by alias
      // 'update' in alias or name
      lu = str::containsCI( la, update ) || str::containsCI( lhs.name(), update );
      ru = str::containsCI( ra, update ) || str::containsCI( rhs.name(), update );
      if ( lu != ru )
        return lu;    // update in alias or name wins
      return la < ra; // finally by alias
    } );
    return std::move(list_r);
  }

} // namespace

RefreshRepoCmd::RefreshRepoCmd(std::vector<std::string> &&commandAliases_r )
  : ZypperBaseCommand (
      std::move( commandAliases_r ),
      _("refresh (ref) [ALIAS|#|URI] ..."),
      // translators: command summary: refresh, ref
      _("Refresh all repositories."),
      // translators: command description
      std::string(_("Refresh repositories specified by their alias, number or URI. If none are specified, all enabled repositories will be refreshed.")) + "\n\n" +
      _("HINT: 'zypper -vv ref' will show the mirrors used to download the metadata.")
      ,
      ResetRepoManager
  )
{

}

std::vector<BaseCommandConditionPtr> RefreshRepoCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}

zypp::ZyppFlags::CommandGroup RefreshRepoCmd::cmdOptions() const
{
  auto that = const_cast<RefreshRepoCmd *>(this);
  return {{
      {"force",'f', ZyppFlags::NoArgument,
            ZyppFlags::BitFieldType( that->_flags, Force ),
            // translators: -f, --force
            _("Force a complete refresh.")
      },
      {"force-build", 'b', ZyppFlags::NoArgument,
            ZyppFlags::BitFieldType( that->_flags, ForceBuild ),
            // translators: -b, --force-build
            _("Force rebuild of the database.")
      },
      {"force-download", 'd', ZyppFlags::NoArgument,
            ZyppFlags::BitFieldType( that->_flags, ForceDownload ),
            // translators: -d, --force-download
            _("Force download of raw metadata.")
      },
      {"build-only", 'B', ZyppFlags::NoArgument,
            ZyppFlags::BitFieldType( that->_flags, BuildOnly ),
            // translators: -B, --build-only
            _("Only build the database, don't download metadata.")
      },
      {"download-only", 'D', ZyppFlags::NoArgument,
            ZyppFlags::BitFieldType( that->_flags, DownloadOnly ),
            // translators: -D, --download-only
            _("Only download raw metadata, don't build the database.")
      },
      {"include-all-archs", 0, ZyppFlags::NoArgument,
            ZyppFlags::BitFieldType( that->_flags, AllArchs ),
            // translators: --include-all-archs
            _("Multi-Arch repos: Download raw metadata for all offered architectures even if the repo supports filtering.")
      },
      {"repo", 'r', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable,
            ZyppFlags::StringVectorType( &that->_repos, ARG_REPOSITORY),
            // translators: -r, --repo <ALIAS|#|URI>
            _("Refresh only specified repositories.")
      }
      ,
      {"services", 's', ZyppFlags::NoArgument,
            ZyppFlags::BoolType( &that->_services, ZyppFlags::StoreTrue, _services ),
            // translators: -s, --services
            _("Refresh also services before refreshing repos.")
      },
  }};
}

void RefreshRepoCmd::doReset()
{
  _flags = Default;
  _repos.clear();
  _services = false;
}

int RefreshRepoCmd::execute( Zypper &zypper , const std::vector<std::string> &positionalArgs_r )
{
  if ( zypper.config().no_refresh )
    zypper.out().warning( str::Format(_("The '%s' global option has no effect here.")) % "--no-refresh" );

  bool force = _flags.testFlag(Force);

  if ( _services )
  {
    if ( !positionalArgs_r.empty() )
    {
      zypper.out().error(str::form(_("Arguments are not allowed if '%s' is used."), "--services"));
      return ( ZYPPER_EXIT_ERR_PRIVILEGES );
    }

    RefreshServicesCmd refServiceCmd( {} );
    refServiceCmd.setForce( force );
    refServiceCmd.setRestoreStatus( false );
    refServiceCmd.setWithRepos( false );

    // needed to be able to retrieve target distribution
    int code = defaultSystemSetup ( zypper, InitTarget );
    if ( code != ZYPPER_EXIT_OK )
      return code;

    zypper.configNoConst().rm_options.servicesTargetDistro = God->target()->targetDistribution();

    code = refServiceCmd.refreshServices( zypper );
    if ( code != ZYPPER_EXIT_OK )
      return code;
  }
  else
  {
    RepoManager::RefreshServiceOptions opts;
    if ( force )
      opts |= RepoManager::RefreshService_forceRefresh;

    checkIfToRefreshPluginServices( zypper, opts );
  }

  MIL << "going to refresh repositories" << endl;
  // need gpg keys when downloading (#304672)
  int code = defaultSystemSetup ( zypper, InitTarget );
  if ( code != ZYPPER_EXIT_OK )
    return code;

  std::vector<std::string> specifiedRepos = _repos;
  for ( const std::string &repoFromCLI : positionalArgs_r )
    specifiedRepos.push_back(repoFromCLI);

  return refreshRepositories ( zypper, _flags, specifiedRepos );
}

bool RefreshRepoCmd::refreshRepository(Zypper &zypper, const RepoInfo &repo, RefreshFlags flags_r)
{
  MIL << "going to refresh repo '" << repo.alias() << "'" << endl;

  // https://code.opensuse.org/leap/features/issue/193
  // https://github.com/openSUSE/zypper/issues/598
  // --include-all-archs is a stub for SUMA(MLM) in case #193 is implemented
  // and honoured by libzypp. Actually we'd prefer using the single-arch
  // repos rather than filtering, but we will see how Leap evolves.
#warning RefreshFlagsBits::AllArchs is a NO-OP - propagate it to libzypp

  // raw metadata refresh
  bool error = false;
  if ( !flags_r.testFlag(BuildOnly) )
  {
    bool force_download = flags_r.testFlag(Force) || flags_r.testFlag(ForceDownload);
    MIL << "calling refreshMetadata" << (force_download ? ", forced" : "") << endl;
    error = refresh_raw_metadata( zypper, repo, force_download );
  }

  // db rebuild
  if ( !( error || flags_r.testFlag(DownloadOnly) ) )
  {
    bool force_build = flags_r.testFlag(Force) || flags_r.testFlag(ForceBuild);;
    MIL << "calling buildCache" << (force_build ? ", forced" : "") << endl;
    error = build_cache( zypper, repo, force_build );
  }

  return error;
}

int RefreshRepoCmd::refreshRepositories( Zypper &zypper, RefreshFlags flags_r, const std::vector<std::string> repos_r )
{
  RepoManager & manager( zypper.repoManager() );
  // bsc#1234752: Try to refresh update repos first (to have updated GPG keys on the fly)
  const std::list<RepoInfo> & repos { inRefreshOrder( manager.knownRepositories() ) };

  // get the list of repos specified on the command line ...
  std::list<RepoInfo> specified;
  std::list<std::string> not_found;
  get_repos( zypper, repos_r.begin(),repos_r.end(), specified, not_found );
  report_unknown_repos( zypper.out(), not_found );

  // --plus-content: It either specifies a known repo (by #, alias or URL)
  // or we need to also refresh all disabled repos to get their content
  // keywords.
  std::set<RepoInfo> plusContent;
  bool doContentCheck = false;
  for ( const std::string & spec : zypper.runtimeData().plusContentRepos )
  {
    RepoInfo r;
    if ( match_repo( zypper, spec, &r ) )
      plusContent.insert( r );	// specific repo: add to plusContent
    else if ( ! doContentCheck )
      doContentCheck = true;	// keyword: need to scan all disabled repos
  }

  std::ostringstream s;
  s << _("Specified repositories: ");
  for_( it, specified.begin(), specified.end() )
    s << it->alias() << " ";
  zypper.out().info( s.str(), Out::HIGH );

  unsigned error_count = 0;
  unsigned enabled_repo_count = repos.size();

  if ( !specified.empty() || not_found.empty() )
  {
    for_( rit, repos.begin(), repos.end() )
    {
      const RepoInfo & repo( *rit );

      if ( repo.enabled() )
      {
        // enabled: Refreshed unless restricted by CLI args or mentioned in
        // --plus-content as specific repo.
        if ( !specified.empty() && std::find( specified.begin(), specified.end(), repo ) == specified.end() )
        {
          if ( plusContent.count( repo ) )
          {
            MIL << "[--plus-content] check " << repo.alias() << endl;
            zypper.out().info( str::Format(_("Refreshing repository '%s'.")) % repo.asUserString(),
                               " [--plus-content]" );
          }
          else
          {
            DBG << repo.alias() << "(#" << ") not specified," << " skipping." << endl;
            enabled_repo_count--;
            continue;
          }
        }
      }
      else
      {
        // disabled: No refresh unless mentioned in --plus-content (specific or content check).
        // CLI args reffering to disabled repos are reported as error.
        if ( doContentCheck || plusContent.count( repo ) )
        {
          MIL << "[--plus-content] check " << repo.alias() << endl;
          zypper.out().info( str::Format(_("Scanning content of disabled repository '%s'.")) % repo.asUserString(),
                             " [--plus-content]" );
        }
        else
        {
          if ( !specified.empty() && std::find( specified.begin(), specified.end(), repo ) == specified.end() )
          {
            DBG << repo.alias() << "(#" << ") not specified," << " skipping." << endl;
          }
          else
          {
            std::string msg( str::Format(_("Skipping disabled repository '%s'")) % repo.asUserString() );

            if ( specified.empty() )
              zypper.out().info( msg, Out::HIGH );
            else
              zypper.out().error( msg );
          }
          enabled_repo_count--;
          continue;
        }
      }

      // do the refresh
      if ( refreshRepository( zypper, repo, flags_r ) )
      {
        zypper.out().error( str::Format(_("Skipping repository '%s' because of the above error.")) % repo.asUserString() );
        ERR << "Skipping repository '" << repo.alias() << "' because of the above error." << endl;
        error_count++;
      }
    }
  }
  else
    enabled_repo_count = 0;

  // print the result message
  if ( !not_found.empty() )
  {
      zypper.out().error(_("Some of the repositories have not been refreshed because they were not known.") );
      return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }
  else if ( enabled_repo_count == 0 )
  {
    int code = ZYPPER_EXIT_OK;
    if ( !specified.empty() )
      zypper.out().warning(_("Specified repositories are not enabled or defined.") );
    else {
      zypper.out().warning(_("There are no enabled repositories defined.") );
      code = ( ZYPPER_EXIT_NO_REPOS );
    }

    zypper.out().info( str::form(_("Use '%s' or '%s' commands to add or enable repositories."),
                                 "zypper addrepo", "zypper modifyrepo" ) );
    return code;
  }
  else if ( error_count == enabled_repo_count )
  {
    zypper.out().error(_("Could not refresh the repositories because of errors.") );
    return ( ZYPPER_EXIT_ERR_ZYPP );
  }
  else if ( error_count )
  {
    zypper.out().error(_("Some of the repositories have not been refreshed because of an error.") );
    return ( ZYPPER_EXIT_ERR_ZYPP );
  }
  else if ( !specified.empty() )
    zypper.out().info(_("Specified repositories have been refreshed.") );
  else
    zypper.out().info(_("All repositories have been refreshed.") );

  return ZYPPER_EXIT_OK;
}
