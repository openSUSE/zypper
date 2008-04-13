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
#include "zypp/ResKind.h"
#include "zypp/PoolItem.h"

#include "zypper.h"
#include "zypper-tabulator.h"
#include "zypper-utils.h"


zypp::ResObject::Kind string_to_kind (const std::string &skind);

/**
 * Run the solver.
 * 
 * \return <tt>true</tt> if a solution has been found, <tt>false</tt> otherwise 
 */
bool resolve(Zypper & zypper);

void install_remove(Zypper & zypper,
                    const Zypper::ArgList & args,
                    bool install_not_remove,
                    const zypp::ResKind & kind);

void mark_by_name (Zypper & zypper,
                   bool install_not_remove,
		   const zypp::ResObject::Kind &kind,
		   const std::string &name);

void mark_by_capability (Zypper & zypper,
                         bool install_not_remove,
			 const zypp::ResObject::Kind &kind,
			 const std::string &capstr);
>>>>>>> fix displaying uninstalled patches:zypper/src/zypper-misc.h

/**
 * Reset all selections made by mark_* methods. Needed in the shell to reset
 * selections after the install and remove commands.
 */
void remove_selections(Zypper & zypper);

/** List patches */
void show_patches(Zypper & zypper);

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
void list_updates(Zypper & zypper,
                  const std::set<zypp::Resolvable::Kind> & kinds,
                  bool best_effort);

/** \todo remove from this header after xu is dropped */ 
bool xml_list_patches();
/** \todo remove from this header after xu is dropped */
void xml_list_updates(const ResKindSet & kinds);

/**
 * \param kind  resolvable type
 * \param skip_interactive whether to skip updates that need user interaction
 * \param best_effort
 */
void mark_updates(const std::set<zypp::Resolvable::Kind> & kinds,
                  bool skip_interactive,
                  bool best_effort);

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
 * Find source packages by names specified as arguments.
 */
void find_src_pkgs(Zypper & zypper);
/**
 * Install source packages found by \ref find_src_pkgs.
 */
void install_src_pkgs(Zypper & zypper);
/**
 * Inject requirements of source packages' build dependencies to the pool.
 */
void build_deps_install(Zypper & zypper);

#endif
