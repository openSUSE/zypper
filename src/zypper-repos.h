#ifndef ZMART_SOURCES_H
#define ZMART_SOURCES_H

#include <boost/logic/tribool.hpp>

#include "zypp/Url.h"

#include "zypper.h"

/**
 * Initialize rpm database on target, if not already initialized. 
 */
void init_target(Zypper & zypper);

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
 * Clean caches for all repositories.
 */
void clean_repos(Zypper & zypper);


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
                     boost::tribool enabled = boost::indeterminate,
                     boost::tribool autorefresh = boost::indeterminate);

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
                        boost::tribool enabled = boost::indeterminate,
                        boost::tribool autorefresh = boost::indeterminate);

/**
 * Add repository specified by \repo to system repositories. 
 */
void add_repo(Zypper & zypper, zypp::RepoInfo & repo);

/**
 * Remove repository specified by \a alias.
 */
bool remove_repo(Zypper & zypper, const std::string &alias );

bool remove_repo(Zypper & zypper,
                 const zypp::Url & url, const zypp::url::ViewOption & urlview);

bool remove_repo(Zypper & zypper, const zypp::RepoInfo & repoinfo);

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
 * Load both repository and target resolvables.
 *
 * \see load_repo_resolvables(bool)
 * \see load_target_resolvables(bool)
 */
void cond_load_resolvables(Zypper & zypper);

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
