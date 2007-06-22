#ifndef ZMART_SOURCES_H
#define ZMART_SOURCES_H

#include <boost/logic/tribool.hpp>

#include "zypp/Url.h"

/**
 * Reads known enabled repositories and stores them in gData.
 * This command also refreshes repos with auto-refresh enabled.
 */
void init_repos();

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
 * \param enabled
 * \param autorefresh
 * \return ZYPPER_EXIT_ERR_ZYPP on unexpected zypp exception,
 *         ZYPPER_EXIT_OK otherwise
 */
int add_repo_by_url( const zypp::Url & url,
                      const std::string & alias,
                      const std::string & type = "",
                      bool enabled = true, bool autorefresh = true );

/**
 * Add repository specified in given repo file on \a repo_file_url.
 * 
 * \param repo_file_url Valid URL of the repo file.
 * \param enabled       If determined, overrides repo file's enabled setting.
 * \param autorefresh   If determined, overrides repo file's autorefresh setting.
 * \return ZYPPER_EXIT_ERR_ZYPP on unexpected zypp exception,
 *         ZYPPER_EXIT_OK otherwise
 */
int add_repo_from_file(const std::string & repo_file_url,
                        boost::tribool enabled = true, boost::tribool autorefresh = true);

/**
 * If ZMD process found, notify user that ZMD is running and that changes
 * to repositories will not be synchronized with it. To be used with commands
 * manipulating repositories like <tt>addrepo</tt> or <tt>rmrepo</tt>. 
 */
void warn_if_zmd();


//! calls init_system_sources if not disabled by user (or non-root)
//void cond_init_system_sources(); // OLD
void init_system_sources(); // OLD

/**
 * Delte repository specified by \a alias.
 */
void remove_repo( const std::string &alias );


/**
 * Rename repository specified by \a alias to \a newalias.
 */
void rename_repo(const std::string & alias, const std::string & newalias);

//void include_source_by_url( const zypp::Url &url ); // OLD

#endif
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
