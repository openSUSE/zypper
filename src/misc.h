/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZMART_MISC_H
#define ZMART_MISC_H

#include <string>

#include "Zypper.h"


/**
 * Loops through resolvables, checking if there is license to confirm. When
 * run interactively, it displays a dialog, otherwise it answers automatically
 * according to --auto-agree-with-licenses present or not present.
 * 
 * \returns true if all licenses have been confirmed, false otherwise.  
 */
bool confirm_licenses(Zypper & zypper);

/**
 * Prints a report about licenses and EULAs of installed packages to stdout.
 */
void report_licenses(Zypper & zypper);

zypp::ResKind string_to_kind (const std::string & skind);

/**
 * Reset all selections made by mark_* methods. Needed in the shell to reset
 * selections after the install and remove commands.
 */
void remove_selections(Zypper & zypper);


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
