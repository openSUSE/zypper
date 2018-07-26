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
 * Commonly shared Options mixin for all commands that install packages
 */
struct InstallerBaseOptions : public OptionsMixin
{
  int _allowDowngrade		= -1;	///< indeterminate: keep the resolvers default
  int _allowNameChange		= -1;	///< indeterminate: keep the resolvers default
  int _allowArchChange		= -1;	///< indeterminate: keep the resolvers default
  int _allowVendorChange	= -1;	///< indeterminate: keep the resolvers default
};

/**
 * Commonly shared Options type for all commands that install packages.
 */
template <const ZypperCommand& TZypperCommand>
struct InstallCommandOptions : public MixinOptions<TZypperCommand, InstallerBaseOptions>
{};

struct InstallOptions : public InstallCommandOptions<ZypperCommand::INSTALL>
{};

struct UpdateOptions : public InstallCommandOptions<ZypperCommand::UPDATE>
{};

struct PatchOptions : public InstallCommandOptions<ZypperCommand::PATCH>
{};

struct VerifyOptions : public InstallCommandOptions<ZypperCommand::VERIFY>
{};

struct InrOptions : public InstallCommandOptions<ZypperCommand::INSTALL_NEW_RECOMMENDS>
{};

struct DupOptions : public InstallCommandOptions<ZypperCommand::DIST_UPGRADE>
{};


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
