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

bool readBoolAnswer();
zypp::ResObject::Kind string_to_kind (const std::string &skind);
void mark_for_install( const zypp::ResObject::Kind &kind,
		       const std::string &name );
void mark_for_uninstall( const zypp::ResObject::Kind &kind,
			 const std::string &name );
int show_summary();
std::string calculate_token();

/**
 * Load both repository and target resolvables into the pool respecting
 * user defined conditions.
 */
void cond_load_resolvables ();

/**
 * Load resolvables from all repositories into the pool. 
 */
void load_target_resolvables();

/**
 * Load installed resolvables from target into the pool.
 */
void load_repo_resolvables();

void establish ();
bool resolve();
void dump_pool ();
void show_patches();
void patch_check();
void list_updates( const zypp::ResObject::Kind &kind );
void mark_updates( const zypp::ResObject::Kind &kind, bool skip_interactive );
void usage(int argc, char **argv);
int solve_and_commit (bool non_interactive = false);
bool confirm_licenses(bool non_interactive = false);

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

#endif
