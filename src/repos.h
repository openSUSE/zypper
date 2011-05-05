/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZMART_SOURCES_H
#define ZMART_SOURCES_H

#include <list>

#include "zypp/TriBool.h"
#include "zypp/Url.h"
#include "zypp/RepoInfo.h"
#include "zypp/ServiceInfo.h"

#include "Zypper.h"

#define  TMP_RPM_REPO_ALIAS  "_tmpRPMcache_"

/**
 * The same as \ref init_repos(), but allows to specify repos to initialize.
 *
 * \param zypper    The zypper instance.
 * \param container A string container of identifier (alias|#|URI) to init.
  */
template <typename Container>
void init_repos(Zypper & zypper, const Container & container = Container());

template<typename T>
void get_repos(Zypper & zypper,
               const T & begin, const T & end,
               std::list<zypp::RepoInfo> & repos, std::list<std::string> & not_found);

void report_unknown_repos(Out & out, std::list<std::string> not_found);

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
 * \returns        Number of repos found.
 *
 * \throws ExitRequestException if one of the repos could not be found.
 */
unsigned repo_specs_to_aliases(
    Zypper & zypper,
    const std::list<std::string> & repos,
    std::list<std::string> & aliases,
    bool enabled_only = true);

/**
 * Reads known enabled repositories and stores them in gData.
 * This command also refreshes repos with auto-refresh enabled.
 *
 * sets exit status to
 *  - ZYPPER_EXIT_ERR_INVALID_ARGS if --repo does not specify a valid repository,
 *  - ZYPPER_EXIT_ERR_ZYPP on error
 */
void init_repos(Zypper & zypper);

/**
 * List defined repositories.
 */
void list_repos(Zypper & zypper);

/**
 * Refresh all enabled repositories.
 */
void refresh_repos(Zypper & zypper);

/**
 * Refresh a single repository.
 * \return true on error, false otherwise
 */
bool refresh_repo(Zypper & zypper, const zypp::RepoInfo & repo);


/**
 * Clean caches for all (specified) repositories.
 */
void clean_repos(Zypper & zypper);

/**
 * Try match given string with any known repository.
 *
 * \param str string to match
 * \param repo pointer to fill with found repository
 * \return success if respository is found
 */
bool match_repo(Zypper & zypper, const std::string str, zypp::RepoInfo *repo = 0);


/**
 * Add repository specified by \url to system repositories.
 *
 * \param url Valid URL of the repository.
 * \param alias
 * \param type
 * \param enabled     Whether the repo should be enabled
 * \param autorefresh Whether the repo should have autorefresh turned on
 */
void add_repo_by_url(Zypper & zypper,
                     const zypp::Url & url,
                     const std::string & alias,
                     const std::string & type = "",
                     zypp::TriBool enabled = boost::indeterminate,
                     zypp::TriBool autorefresh = boost::indeterminate,
                     zypp::TriBool keepPackages = boost::indeterminate,
                     zypp::TriBool gpgCheck  = boost::indeterminate);

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
                        const std::string & repo_file_url,
                        zypp::TriBool enabled = boost::indeterminate,
                        zypp::TriBool autorefresh = boost::indeterminate,
                        zypp::TriBool keepPackages = boost::indeterminate,
                        zypp::TriBool gpgCheck  = boost::indeterminate);


/**
 * Add repository specified by \repo to system repositories.
 */
void add_repo(Zypper & zypper, zypp::RepoInfo & repo);

/**
 * Remove repository specified by \a alias.
 */
void remove_repo(Zypper & zypper, const zypp::RepoInfo & repoinfo);

/**
 * Rename repository specified by \a alias to \a newalias.
 */
void rename_repo(Zypper & zypper,
                 const std::string & alias, const std::string & newalias);

/**
 * Modify repository properties.
 *
 * \param alias repository alias
 */
void modify_repo(Zypper & zypper, const std::string & alias);

/**
 * Modify all repositories properties.
 *
 */
void modify_all_repos(Zypper & zypper);

/**
 * Modify repositories which is matching filter options
 * like all, local, remote or medium-type
 */
void modify_repos_by_option( Zypper & zypper );


void list_services(Zypper & zypper);

void add_service(Zypper & zypper, const zypp::ServiceInfo & service);

void add_service_by_url( Zypper & zypper,
                         const zypp::Url & url, const std::string & alias,
                         const std::string & type,
                         boost::tribool enabled = boost::indeterminate);

void remove_service(Zypper & zypper, const zypp::ServiceInfo & service);

void modify_service(Zypper & zypper, const std::string & alias);

void refresh_services(Zypper & zypper);

bool match_service(Zypper & zypper,
                   std::string str,
                   zypp::repo::RepoInfoBase_Ptr & service_ptr);

void modify_services_by_option( Zypper & zypper );

/**
 * Initialize rpm database on target, if not already initialized.
 */
void init_target(Zypper & zypper);

/**
 * Load both repository and target resolvables.
 *
 * \see load_repo_resolvables(bool)
 * \see load_target_resolvables(bool)
 */
void load_resolvables(Zypper & zypper);

/**
 * Reads resolvables from the RPM database (installed resolvables) into the pool.
 *
 */
void load_target_resolvables(Zypper & zypper);

/**
 * Reads resolvables from the repository solv cache.
 */
void load_repo_resolvables(Zypper & zypper);


/**
 * If ZMD process found, notify user that ZMD is running and that changes
 * to repositories will not be synchronized with it. To be used with commands
 * manipulating repositories like <tt>addrepo</tt> or <tt>rmrepo</tt>.
 */
void warn_if_zmd();

#endif
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
