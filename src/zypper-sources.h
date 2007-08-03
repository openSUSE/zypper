#ifndef ZMART_SOURCES_H
#define ZMART_SOURCES_H

#include "zypp/Url.h"
#include <boost/logic/tribool.hpp>
#include "zypper-getopt.h"

/**
 * Reads known enabled repositories and stores them in gData.
 * This command also refreshes repos with auto-refresh enabled.
 * 
 * \return ZYPPER_EXIT_ERR_INVALID_ARGS if --repo does not specify a valid repository,
 *         ZYPPER_EXIT_ERR_ZYPP on error, ZYPPER_EXIT_OK otherwise.
 */
int init_repos();

/**
 * List defined repositories.
 */
void list_repos();

/**
 * Refresh all enabled repositories.
 */
void refresh_repos();


/**
 * Add repository specified by \url to system repositories.
 * 
 * \param url Valid URL of the repository.
 * \param alias
 * \param type
 * \param enabled     Whether the repo should be enabled   
 * \param autorefresh Whether the repo should have autorefresh turned on
 * \return ZYPPER_EXIT_ERR_ZYPP on unexpected zypp exception,
 *         ZYPPER_EXIT_OK otherwise
 */
int add_repo_by_url( const zypp::Url & url,
                      const std::string & alias,
                      const std::string & type = "",
                      boost::tribool enabled = boost::indeterminate, boost::tribool autorefresh = boost::indeterminate);

/**
 * Add repository specified in given repo file on \a repo_file_url. All repos
 * will be added enabled and with autorefresh turned on. The enabled and
 * autorefresh values provided in the files will be ignored.
 * 
 * \param repo_file_url Valid URL of the repo file.
 * \param enabled     Whether the repo should be enabled   
 * \param autorefresh Whether the repo should have autorefresh turned on
 * \return ZYPPER_EXIT_ERR_ZYPP on unexpected zypp exception,
 *         ZYPPER_EXIT_OK otherwise
 */
int add_repo_from_file(const std::string & repo_file_url,
                       boost::tribool enabled = boost::indeterminate, boost::tribool autorefresh = boost::indeterminate);

/**
 * If ZMD process found, notify user that ZMD is running and that changes
 * to repositories will not be synchronized with it. To be used with commands
 * manipulating repositories like <tt>addrepo</tt> or <tt>rmrepo</tt>. 
 */
void warn_if_zmd();

/**
 * Delte repository specified by \a alias.
 */
bool remove_repo( const std::string &alias );

bool remove_repo( const zypp::Url & url, const zypp::url::ViewOption & urlview );

/**
 * Rename repository specified by \a alias to \a newalias.
 */
void rename_repo(const std::string & alias, const std::string & newalias);


/**
 * Modify repository properties.
 * 
 * \param alias repository alias
 */
void modify_repo(const std::string & alias);

#endif
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
