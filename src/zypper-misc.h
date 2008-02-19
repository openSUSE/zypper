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
#include "zypp/ResObject.h"
#include "zypp/PoolItem.h"

#include "zypper-tabulator.h"

/**
 * Initialize rpm database on target, if not already initialized. 
 */
void cond_init_target(Zypper & zypper);

/// Parse a capability string. On error print a message and return noCap
zypp::Capability safe_parse_cap (const Zypper & zypper,
                                 const zypp::ResObject::Kind &kind,
				 const std::string &capstr);

zypp::ResObject::Kind string_to_kind (const std::string &skind);
void mark_for_install(Zypper & zypper,
                      const zypp::ResObject::Kind &kind,
		      const std::string &name);

void mark_for_uninstall(Zypper & zypper,
                        const zypp::ResObject::Kind &kind,
			const std::string &name);

void mark_by_name (Zypper & zypper,
                   bool install_not_remove,
		   const zypp::ResObject::Kind &kind,
		   const std::string &name);

void mark_by_capability (const Zypper & zypper,
                         bool install_not_remove,
			 const zypp::ResObject::Kind &kind,
			 const std::string &capstr);

/**
 * Reset all selections made by mark_* methods. Needed in the shell to reset
 * selections after the install and remove commands.
 */
void remove_selections(Zypper & zypper);

int show_summary(Zypper & zypper);
//std::string calculate_token();

void establish();
bool resolve(Zypper & zypper);
void dump_pool();
void show_patches(Zypper & zypper);
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
void list_updates(const Zypper & zypper,
                  const zypp::ResObject::Kind &kind,
                  bool best_effort);

/**
 * \param kind  resolvable type
 * \param skip_interactive whether to skip updates that need user interaction
 * \param best_effort
 */
void mark_updates( const zypp::ResObject::Kind &kind, bool skip_interactive, bool best_effort );

/**
 * Runs solver on the pool, asks to choose solution of eventual problems
 * (when run interactively) and commits the result.
 * 
 * \param have_extra_deps ?
 * \return ZYPPER_EXIT_INF_REBOOT_NEEDED, ZYPPER_EXIT_INF_RESTART_NEEDED,
 *         or ZYPPER_EXIT_OK or ZYPPER_EXIT_ERR_ZYPP on zypp erorr. 
 *  
 */
void solve_and_commit(Zypper & zypper);

/**
 * Loops through resolvables, checking if there is license to confirm. When
 * run interactively, it displays a dialog, otherwise it answers automatically
 * according to --auto-agree-with-licenses present or not present.
 * 
 * \returns true if all licenses have been confirmed, false otherwise.  
 */
bool confirm_licenses(Zypper & zypper);

// copied from yast2-pkg-bindings:PkgModuleFunctions::DoProvideNameKind
struct ProvideProcess
{
  zypp::PoolItem item;
  zypp::PoolItem installed_item;
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
int build_deps_install(std::vector<std::string> & arguments);

#endif

