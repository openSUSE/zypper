/*-----------------------------------------------------------*- c++ -*-\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZMART_MISC_H
#define ZMART_MISC_H

#include <string>
#include "zypp/Url.h"
#include "zypp/ResObject.h"
#include "zypp/PoolItem.h"
#include "zypper-tabulator.h"

/**
 * Initialize rpm database on target, if not already initialized. 
 */
void cond_init_target();

/// Parse a capability string. On error print a message and return noCap
zypp::Capability safe_parse_cap (const zypp::ResObject::Kind &kind,
				 const std::string &capstr);

zypp::ResObject::Kind string_to_kind (const std::string &skind);
void mark_for_install( const zypp::ResObject::Kind &kind,
		       const std::string &name );
void mark_for_uninstall( const zypp::ResObject::Kind &kind,
			 const std::string &name );

void mark_by_name (bool install_not_delete,
		   const zypp::ResObject::Kind &kind,
		   const std::string &name );
void mark_by_capability (bool install_not_delete,
			 const zypp::ResObject::Kind &kind,
			 const std::string &capstr );

int show_summary();
//std::string calculate_token();

/**
 * Load both repository and target resolvables.
 * 
 * \param to_pool If <tt>true</tt>, the resolvables are added to the pool, if
 *        <tt>false</tt> they will be stored in \ref gData.repo_resolvalbes
 *        and \ref gData.target_resolvables (global ResStore vector).
 * 
 * \see load_repo_resolvables(bool)
 * \see load_target_resolvables(bool)
 */
void cond_load_resolvables(bool to_pool = true);

/**
 * Reads resolvables from the RPM database (installed resolvables) into the pool.
 * 
 * \param to_pool If <tt>true</tt>, the resolvables are added to the pool, if
 *        <tt>false</tt> they will be stored \ref gData.target_resolvables
 *        (global ResStore variable).
 */
void load_target_resolvables(bool to_pool = true);

/**
 * Reads resolvables from the repository sqlite cache. 
 * 
 * \param to_pool If <tt>true</tt>, the resolvables are added to the pool, if
 *        <tt>false</tt> they will be stored in \ref gData.repo_resolvables
 *        (global ResStore vector).
 */
void load_repo_resolvables(bool to_pool = true);

void establish ();
bool resolve();
void dump_pool ();
void show_patches();
bool xml_list_patches();
void xml_list_updates();


/**
 * Are there applicable patches?
 */
void patch_check();

/**
 * Lists available updates of installed resolvables of specified \a kind.
 * if repo_alias != "", restrict updates to this repository.
 * if best_effort == true, any version greater than the installed one will do.
 * Prints the table of updates to stdout.
 * 
 * \param kind  resolvable type
 * \param best_effort
 */
void list_updates( const zypp::ResObject::Kind &kind, bool best_effort );

/**
 * \param kind  resolvable type
 * \param skip_interactive whether to skip updates that need user interaction
 * \param best_effort
 */
void mark_updates( const zypp::ResObject::Kind &kind, bool skip_interactive, bool best_effort );

/**
 * 
 */
void usage(int argc, char **argv);

/**
 * Runs solver on the pool, asks to choose solution of eventual problems
 * (when run interactively) and commits the result.
 * 
 * \param have_extra_deps ?
 * \return ZYPPER_EXIT_INF_REBOOT_NEEDED, ZYPPER_EXIT_INF_RESTART_NEEDED,
 *         or ZYPPER_EXIT_OK or ZYPPER_EXIT_ERR_ZYPP on zypp erorr. 
 *  
 */
int solve_and_commit ();

/**
 * Loops through resolvables, checking if there is license to confirm. When
 * run interactively, it displays a dialog, otherwise it answers automatically
 * according to --auto-agree-with-licenses present or not present.
 * 
 * \returns true if all licenses have been confirmed, false otherwise.  
 */
bool confirm_licenses();

// copied from yast2-pkg-bindings:PkgModuleFunctions::DoProvideNameKind
struct ProvideProcess
{
  zypp::PoolItem_Ref item;
  zypp::PoolItem_Ref installed_item;
  zypp::ResStatus::TransactByValue whoWantsIt;
  std::string version;
  zypp::Arch _architecture;

  ProvideProcess(zypp::Arch arch, const std::string &vers)
    : whoWantsIt(zypp::ResStatus::USER)
    , version(vers)
    , _architecture( arch )
    { }

  bool operator()( const zypp::PoolItem& provider );
};

/**
 * Installs source packages specified by name.
 */
int source_install(std::vector<std::string> & arguments);

#endif
