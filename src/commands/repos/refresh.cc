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

#include <zypp-core/base/UserRequestException>

#include <cerrno>
#include <cstring>
#include <limits.h>
#include <map>
#include <sys/wait.h>
#include <unistd.h>

using namespace zypp;

extern ZYpp::Ptr God;

namespace {
  /** Repositories expected to provide updated GPG keys (bsc#1234752).
   *  They must be refreshed before the other repositories.
   *  \li 2: 'update' directory within the repo's path
   *  \li 1: 'update' in alias or name
   */
  unsigned updateRepoRank( const RepoInfo & repo_r )
  {
    static const std::string update { "update" };
    if ( str::containsCI( repo_r.url().getPathName(), update ) )
      return 2;
    if ( str::containsCI( repo_r.alias(), update ) || str::containsCI( repo_r.name(), update ) )
      return 1;
    return 0;
  }

  // bsc#1234752: Try to refresh update repos first (to have updated GPG keys on the fly).
  // GA repos usually ship the old, maybe meanwhile expired, GPG key. If such a key was
  // prolonged, the update repo may contain it and zypp updates the trusted key on the fly
  // when refreshing it. This avoids a 'key has expired' warning being issued when refreshing
  // the GA repos
  inline std::list<RepoInfo> inRefreshOrder( std::list<RepoInfo> && list_r )
  {
    list_r.sort( []( const RepoInfo & lhs, const RepoInfo & rhs ) {
      const unsigned lrank = updateRepoRank( lhs );
      const unsigned rrank = updateRepoRank( rhs );
      if ( lrank != rrank )
        return lrank > rrank;
      return lhs.alias() < rhs.alias();
    } );
    return std::move(list_r);
  }

  /** Path of the running zypper executable. */
  const std::string & selfExecutable()
  {
    static const std::string exe = []() {
      char buf[ PATH_MAX + 1 ] = { 0 };
      ssize_t len = ::readlink( "/proc/self/exe", buf, PATH_MAX );
      return std::string( len > 0 ? buf : "/usr/bin/zypper" );
    }();
    return exe;
  }

  /** Fork a child process refreshing a single repository.
   *  \return child pid, -1 on error. */
  pid_t startRefreshChild( const Zypper & zypper, const RepoInfo & repo_r, RefreshRepoCmd::RefreshFlags flags_r )
  {
    std::vector<std::string> args { selfExecutable() };
    for ( const std::string & globalOpt : zypper.globalOptions() )
      args.push_back( globalOpt );
    args.push_back( "--internal-refresh-child" );
    args.push_back( "refresh" );
    args.push_back( "--jobs" );
    args.push_back( "1" );
    if ( flags_r.testFlag( RefreshRepoCmd::Force ) )
      args.push_back( "--force" );
    if ( flags_r.testFlag( RefreshRepoCmd::ForceBuild ) )
      args.push_back( "--force-build" );
    if ( flags_r.testFlag( RefreshRepoCmd::ForceDownload ) )
      args.push_back( "--force-download" );
    if ( flags_r.testFlag( RefreshRepoCmd::DownloadOnly ) )
      args.push_back( "--download-only" );
    if ( flags_r.testFlag( RefreshRepoCmd::AllArchs ) )
      args.push_back( "--include-all-archs" );
    args.push_back( repo_r.alias() );

    std::vector<char *> argv;
    argv.reserve( args.size() + 1 );
    for ( std::string & arg : args )
      argv.push_back( const_cast<char *>( arg.c_str() ) );
    argv.push_back( nullptr );

    // The parallel refresh parent holds the ZYpp lock. Children work on
    // distinct repositories and must not try to acquire it.
    std::vector<std::string> envStore;
    bool haveReadonlyHack = false;
    for ( char ** env = environ; *env; ++env )
    {
      std::string entry( *env );
      if ( entry.rfind( "ZYPP_READONLY_HACK=", 0 ) == 0 )
      {
        entry = "ZYPP_READONLY_HACK=1";
        haveReadonlyHack = true;
      }
      envStore.push_back( std::move( entry ) );
    }
    if ( !haveReadonlyHack )
      envStore.emplace_back( "ZYPP_READONLY_HACK=1" );

    std::vector<char *> envp;
    envp.reserve( envStore.size() + 1 );
    for ( std::string & entry : envStore )
      envp.push_back( const_cast<char *>( entry.c_str() ) );
    envp.push_back( nullptr );

    const std::string & exe = selfExecutable();	// initialized before fork()
    const pid_t pid = ::fork();
    if ( pid == 0 )
    {
      ::execve( exe.c_str(), argv.data(), envp.data() );
      ::_exit( 127 );
    }
    return pid;
  }

  /** Refresh \a repos_r each in its own child process, at most \a jobs_r at a time.
   *  \return number of failed repositories */
  unsigned runRefreshChildren( Zypper & zypper, const std::vector<RepoInfo> & repos_r,
                               RefreshRepoCmd::RefreshFlags flags_r, unsigned jobs_r )
  {
    if ( repos_r.empty() )
      return 0;

    unsigned error_count = 0;
    std::vector<unsigned char> failed( repos_r.size(), 0 );
    std::map<pid_t, size_t> running;
    size_t next = 0;
    bool aborted = false;

    auto reportFailure = [&]( size_t idx ) {
      if ( failed[idx] )
        return;
      failed[idx] = 1;
      zypper.out().error( str::Format(_("Skipping repository '%s' because of the above error.")) % repos_r[idx].asUserString() );
      ERR << "Skipping repository '" << repos_r[idx].alias() << "' because of the above error." << endl;
      error_count++;
    };

    while ( next < repos_r.size() || !running.empty() )
    {
      if ( zypper.exitRequested() )
        aborted = true;

      while ( !aborted && running.size() < jobs_r && next < repos_r.size() )
      {
        const pid_t pid = startRefreshChild( zypper, repos_r[next], flags_r );
        if ( pid > 0 )
        {
          running[pid] = next++;
        }
        else
        {
          zypper.out().error( str::Format(_("Could not start a refresh process for repository '%s'.")) % repos_r[next].asUserString() );
          reportFailure( next );
          ++next;
        }
      }

      if ( running.empty() )
        break;

      int status = 0;
      const pid_t pid = ::waitpid( -1, &status, 0 );
      if ( pid < 0 )
      {
        if ( errno == EINTR )
          continue;
        ERR << "waitpid failed: " << ::strerror( errno ) << endl;
        break;
      }

      auto it = running.find( pid );
      if ( it == running.end() )
        continue;
      const size_t idx = it->second;
      running.erase( it );

      if ( WIFEXITED( status ) )
      {
        if ( WEXITSTATUS( status ) != 0 )
          reportFailure( idx );
      }
      else
      {
        zypper.out().error( str::Format(_("Refresh process for repository '%s' was terminated unexpectedly.")) % repos_r[idx].asUserString() );
        reportFailure( idx );
      }
    }

    if ( aborted )
    {
      zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
      throw AbortRequestException();
    }
    return error_count;
  }

  /** Number of parallel repository refreshes to use by default. */
  unsigned defaultJobs()
  {
    const long nprocs = ::sysconf( _SC_NPROCESSORS_ONLN );
    return nprocs > 0 ? static_cast<unsigned>( nprocs ) : 1u;
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
      {"jobs", 'j', ZyppFlags::RequiredArgument,
            ZyppFlags::IntType( &that->_jobs ),
            // translators: -j, --jobs <N>
            _("Number of repositories to refresh in parallel. The default is the number of CPUs, limited by the number of repositories. Use '--jobs 1' to refresh sequentially.")
      },
  }};
}

void RefreshRepoCmd::doReset()
{
  _flags = Default;
  _repos.clear();
  _services = false;
  _jobs = 0;
}

int RefreshRepoCmd::execute( Zypper &zypper , const std::vector<std::string> &positionalArgs_r )
{
  if ( zypper.config().no_refresh )
    zypper.out().warning( str::Format(_("The '%s' global option has no effect here.")) % "--no-refresh" );

  if ( _jobs < 0 )
  {
    zypper.out().error( str::Format(_("Invalid number of jobs: '%s'.")) % str::numstring(_jobs) );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }

  // Child processes spawned for parallel refresh must not refresh services
  // again (the parent has already done this) and must not spawn further children.
  const bool refreshChild = zypper.config().internal_refresh_child;

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
  else if ( !refreshChild )
  {
    RepoManager::RefreshServiceOptions opts;
    if ( force )
      opts |= RepoManager::RefreshService_forceRefresh;

    checkIfToAutoRefreshServices( zypper, opts );
  }

  MIL << "going to refresh repositories" << endl;
  // need gpg keys when downloading (#304672)
  int code = defaultSystemSetup ( zypper, InitTarget );
  if ( code != ZYPPER_EXIT_OK )
    return code;

  std::vector<std::string> specifiedRepos = _repos;
  for ( const std::string &repoFromCLI : positionalArgs_r )
    specifiedRepos.push_back(repoFromCLI);

  const unsigned jobs = refreshChild ? 1u : static_cast<unsigned>( _jobs );
  return refreshRepositories ( zypper, _flags, specifiedRepos, jobs );
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
    error = refresh_raw_metadata( zypper, repo, force_download, zypper.config().internal_refresh_child );
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

unsigned RefreshRepoCmd::refreshRepositories( Zypper & zypper, const std::vector<RepoInfo> & repos_r, RefreshFlags flags_r, unsigned jobs_r )
{
  if ( repos_r.empty() )
    return 0;

  unsigned jobs = jobs_r ? jobs_r : defaultJobs();
  jobs = std::min( jobs, static_cast<unsigned>( repos_r.size() ) );

  // Refreshing in parallel child processes is only safe if they do not
  // have to share the parent's state or the output format.
  const bool useChildren = jobs > 1
    && !flags_r.testFlag(BuildOnly)
    && !zypper.runningShell()
    && !zypper.config().machine_readable
    && zypper.runtimeData().plusContentRepos.empty()
    && zypper.config().plusRepoFromCLI.empty();

  if ( !useChildren )
  {
    unsigned error_count = 0;
    for ( const RepoInfo & repo : repos_r )
    {
      if ( refreshRepository( zypper, repo, flags_r ) )
      {
        // A child process reports failures to its parent, see below.
        if ( !zypper.config().internal_refresh_child )
        {
          zypper.out().error( str::Format(_("Skipping repository '%s' because of the above error.")) % repo.asUserString() );
          ERR << "Skipping repository '" << repo.alias() << "' because of the above error." << endl;
        }
        error_count++;
      }
    }
    return error_count;
  }

  // bsc#1234752: refresh repositories providing updated GPG keys first.
  // They are handled sequentially before the others are started in parallel.
  std::vector<RepoInfo> updateRepos;
  std::vector<RepoInfo> otherRepos;
  for ( const RepoInfo & repo : repos_r )
  {
    if ( updateRepoRank( repo ) )
      updateRepos.push_back( repo );
    else
      otherRepos.push_back( repo );
  }

  // bsc#1234752: complete these before starting the parallel refresh of the others
  unsigned error_count = 0;
  for ( const RepoInfo & repo : updateRepos )
  {
    if ( refreshRepository( zypper, repo, flags_r ) )
    {
      zypper.out().error( str::Format(_("Skipping repository '%s' because of the above error.")) % repo.asUserString() );
      ERR << "Skipping repository '" << repo.alias() << "' because of the above error." << endl;
      error_count++;
    }
  }

  error_count += runRefreshChildren( zypper, otherRepos, flags_r, jobs );
  return error_count;
}

int RefreshRepoCmd::refreshRepositories( Zypper &zypper, RefreshFlags flags_r, const std::vector<std::string> repos_r, unsigned jobs_r )
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
  std::vector<RepoInfo> refreshList;

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

      refreshList.push_back( repo );
    }
  }
  else
    enabled_repo_count = 0;

  error_count = refreshRepositories( zypper, refreshList, flags_r, jobs_r );

  // A refresh child reports problems itself. The overall result is
  // reported by the parent process.
  if ( zypper.config().internal_refresh_child )
  {
    if ( !not_found.empty() )
      return ZYPPER_EXIT_ERR_INVALID_ARGS;
    if ( enabled_repo_count == 0 )
      return ZYPPER_EXIT_NO_REPOS;
    if ( error_count )
      return ZYPPER_EXIT_ERR_ZYPP;
    return ZYPPER_EXIT_OK;
  }

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
