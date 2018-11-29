/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZMART_MISC_H
#define ZMART_MISC_H

class Zypper;
using namespace zypp;

/**
 * Loops through resolvables, checking if there is license to confirm. When
 * run interactively, it displays a dialog, otherwise it answers automatically
 * according to \sa LicenseAgreementPolicy
 *
 * \returns true if all licenses have been confirmed, false otherwise.
 */
bool confirm_licenses( Zypper & zypper );

/**
 * Prints a report about licenses and EULAs of installed packages to stdout.
 */
void report_licenses( Zypper & zypper );

/**
 * Reset all selections made by mark_* methods. Needed in the shell to reset
 * selections after the install and remove commands.
 */
void remove_selections( Zypper & zypper );


/**
 * Find source packages by names specified as arguments and enqueue them
 * for installation.
 *
 * \todo this will get deprecated after commit refactoring
 * \note we still need to be able to install the source package alone
 *       (without build-deps, which are listed as 'requires' of the srcpackage)
 */
void mark_src_pkgs( Zypper & zypper );

/**
 * Install source packages found by \ref mark_src_pkgs.
 * \todo this uses ZYpp->installSrcPackage(srcpkg) - if it is to be gone
 *       during the zypp commit refactoring, we need a replacement
 */
void install_src_pkgs( Zypper & zypper );

/**
 * Inject requirements of a source package or its build dependencies (depending
 * on command line options) to the pool.
 */
void build_deps_install( Zypper & zypper );

#endif
