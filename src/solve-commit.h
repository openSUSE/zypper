/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

/**
 * Solve and commit code.
 *
 * This part is responsible for calling the solver, feeding the result back
 * to the user in form of dependency problem dialogue and installation summary,
 * and calling commit to do actually do what has been written in the summary. 
 */
#ifndef SOLVE_COMMIT_H_
#define SOLVE_COMMIT_H_

#include "Zypper.h"

/**
 * Commonly shared Options type for all commands that install packages
 */
struct InstallerBaseOptions : public Options
{
  InstallerBaseOptions ( const ZypperCommand & command_r ) : Options(command_r)
    , _allowDowngrade( -1 )
    , _allowNameChange( -1 )
    , _allowArchChange( -1 )
    , _allowVendorChange( -1 )
  {}
  int _allowDowngrade;
  int _allowNameChange;
  int _allowArchChange;
  int _allowVendorChange;
};

/** dup specific options */
struct DupOptions : public InstallerBaseOptions
{
  DupOptions()
    : InstallerBaseOptions( ZypperCommand::DIST_UPGRADE )
  {}
};

/**
 * Run the solver.
 * 
 * \return <tt>true</tt> if a solution has been found, <tt>false</tt> otherwise 
 */
bool resolve(Zypper & zypper);


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


#endif /*SOLVE_COMMIT_H_*/
