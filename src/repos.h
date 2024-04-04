/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZMART_SOURCES_H
#define ZMART_SOURCES_H

#include <list>

#include <boost/lexical_cast.hpp>

#include <zypp/TriBool.h>
#include <zypp/Url.h>
#include <zypp/RepoInfo.h>
#include <zypp/ServiceInfo.h>

#include "Zypper.h"
#include "commands/reposerviceoptionsets.h"

#define  TMP_RPM_REPO_ALIAS  "_tmpRPMcache_"

// | Enabled | GPG Check |  Colored strings for enabled and GPG Check status
// +---------+-----------+
// | Yes     | (  ) No   |
// | Yes     | (rp) Yes  |
// | No      | ----      |
struct RepoGpgCheckStrings
{
  RepoGpgCheckStrings();

  RepoGpgCheckStrings( const ServiceInfo & service_r );
  RepoGpgCheckStrings( const RepoInfo & repo_r );

  ColorContext _tagColor;	///< color according to enabled and GPG Check status
  ColorString _enabledYN;	///< colored enabled Yes/No
  ColorString _gpgCheckYN;	///< colored GPG Check status if enabled else "----"
};

inline std::string timestamp()
{ return Date::now().form("%Y%m%d-%H%M%S"); }

/**
 * Say "Repository %s not found" for all strings in \a not_found list.
 */
void report_unknown_repos( Out & out, const std::list<std::string> & not_found );

template<typename T>
void get_repos( Zypper & zypper, const T & begin, const T & end, std::list<RepoInfo> & repos, std::list<std::string> & not_found );

template <typename Target, typename Source>
void safe_lexical_cast( Source s, Target & tr )
{
  try
  {
    tr = boost::lexical_cast<Target>( s );
  }
  catch ( boost::bad_lexical_cast & )
  {;}
}

void report_unknown_repos( Out & out, const std::list<std::string> & not_found );

/**
 * Looks for known repos based on specified arguments and creates a list
 * of aliases of found repos. Reports repos not found via zypper's output.
 *
 * By default, only enabled repos are returned. This can be changed via the
 * \a enabled_only argument.
 *
 * \param zypper   The Zypper instance.
 * \param repos    List of repository specifiers as strings (alias/name/#/URL)
 * \param aliases  List of strings which should be fed by found aliases.
 * \param enabled_only Whether to return only enabled repos. Default is true.
 * \returns        ZYPPER_EXIT_OK or zypper error codes
 */
int repo_specs_to_aliases(Zypper & zypper, const std::vector<std::string> &repos,
                                std::list<std::string> & aliases, bool enabled_only = true );

/**
 * Reads known enabled repositories and stores them in gData.
 * This command also refreshes repos with auto-refresh enabled.
 *
 * sets exit status to
 *  - ZYPPER_EXIT_ERR_INVALID_ARGS if --repo does not specify a valid repository,
 *  - ZYPPER_EXIT_ERR_ZYPP on error
 */
void init_repos( Zypper & zypper );

/**
 * List defined repositories.
 */
void list_repos( Zypper & zypper );

/**
 * Clean caches for all (specified) repositories.
 */
enum class CleanRepoBits {
  Default = 0,
  CleanMetaData = 1,
  CleanRawMetaData = 2,
  CleanAll = CleanMetaData | CleanRawMetaData
};
ZYPP_DECLARE_FLAGS_AND_OPERATORS(CleanRepoFlags, CleanRepoBits)
void clean_repos(Zypper & zypper, std::vector<std::string> specificRepos, CleanRepoFlags flags );

/**
 * Try match given string with any known repository.
 *
 * \param str string to match
 * \param repo pointer to fill with found repository
 * \param looseQuery_r Ignore query string in the URI if true.
 * \param looseAuth_r Ignore user authentication data in the URI if true.
 * \return success if respository is found
 */
bool match_repo( Zypper & zypper, const std::string str, RepoInfo *repo = 0 , bool looseQuery_r = false, bool looseAuth_r = false );

/**
 * Add repository specified by \url to system repositories.
 *
 * \param url Valid URL of the repository.
 * \param alias
 * \param enabled     Whether the repo should be enabled
 * \param autorefresh Whether the repo should have autorefresh turned on
 */
void add_repo_by_url(Zypper & zypper,
                      const Url & url,
                      const std::string & alias,
                      const RepoServiceCommonOptions &opts,
                      const RepoProperties &repoProps, bool noCheck);

/**
 * Add repository specified in given repo file on \a repo_file_url. All repos
 * will be added enabled and with autorefresh turned on. The enabled and
 * autorefresh values provided in the files will be ignored.
 *
 * \param repo_file_url Valid URL of the repo file.
 * \param enabled     Whether the repo should be enabled
 * \param autorefresh Whether the repo should have autorefresh turned on
 */
void add_repo_from_file(Zypper & zypper,
                         const std::string & repo_file_url , const RepoServiceCommonOptions &opts, const RepoProperties &repoProps, bool noCheck);

/**
 * Add repository specified by \repo to system repositories.
 */
bool add_repo(Zypper & zypper, RepoInfo & repo , bool noCheck);


/**
 * Remove repository specified by \a alias.
 */
void remove_repo( Zypper & zypper, const RepoInfo & repoinfo );

/**
 * Remove repositories which is matching filter options
 * like all, local, remote or medium-type
 */
void remove_repos_by_option(Zypper & zypper_r , const RepoServiceCommonSelectOptions selOpts_r);

/**
 * Modify repository properties.
 *
 * \param alias repository alias
 */
void modify_repo( Zypper & zypper, const std::string & alias, const RepoServiceCommonOptions &commonOpts, const RepoProperties &repoProps );

/**
 * Modify repositories which is matching filter options
 * like all, local, remote or medium-type
 */
void modify_repos_by_option( Zypper & zypper, const RepoServiceCommonSelectOptions &selectOpts, const RepoServiceCommonOptions &commonOpts, const RepoProperties &repoProps  );

void remove_service( Zypper & zypper, const ServiceInfo & service );

void modify_service( Zypper & zypper, const std::string & alias );

/** If root, refresh any plugin services before lr/ls/ref (bnc#893294) */
void checkIfToRefreshPluginServices( Zypper & zypper, RepoManager::RefreshServiceFlags flags_r =  RepoManager::RefreshServiceFlags() );

void modify_services_by_option( Zypper & zypper );

/**
 * Initialize rpm database on target, if not already initialized.
 */
void init_target( Zypper & zypper );

/**
 * Load both repository and target resolvables.
 *
 * \see load_repo_resolvables(bool)
 * \see load_target_resolvables(bool)
 */
void load_resolvables( Zypper & zypper );

/**
 * Reads resolvables from the RPM database (installed resolvables) into the pool.
 *
 */
void load_target_resolvables( Zypper & zypper );

/**
 * Reads resolvables from the repository solv cache.
 */
void load_repo_resolvables( Zypper & zypper );

ColorString repoPriorityNumber( unsigned prio_r, int width_r = 0 );
ColorString repoPriorityNumberAnnotated( unsigned prio_r, int width_r = 0 );

const char * repoAutorefreshStr( const repo::RepoInfoBase & repo_r );

/** \return true if aliases are equal, and all lhs urls can be found in rhs */
bool repo_cmp_alias_urls( const RepoInfo & lhs, const RepoInfo & rhs );

void repoPrioSummary( Zypper & zypper );

bool refresh_raw_metadata( Zypper & zypper, const RepoInfo & repo, bool force_download );

bool build_cache( Zypper & zypper, const RepoInfo & repo, bool force_build );

/**
 * Iterate over \a positionalArgs and try to treat it as a .rpm file, in case it turns out to be a valid
 * rpm file, remove the arg from the list and place the file in a temporary repository
 */
std::vector<std::string> createTempRepoFromArgs(Zypper &zypper, std::vector<std::string> &positionalArgs , bool allowUnsigned_r);

#endif
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
