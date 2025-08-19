/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_INSTALLREMOVE_INCLUDED
#define ZYPPER_COMMANDS_INSTALLREMOVE_INCLUDED

#include "commands/basecommand.h"
#include "commands/optionsets.h"
#include "commands/solveroptionset.h"
#include "utils/flags/zyppflags.h"
#include "SolverRequester.h"

#include <zypp/ResObject.h>

class InstallRemoveBase : public ZypperBaseCommand
{
public:
  using ZypperBaseCommand::ZypperBaseCommand;

  /** The attempt to provide a common way of handling (104) exit info for all install commands.
   *
   * Indicates 104 unless \c ignore-unknown is set. In \c non-interactive mode
   * this is treated as an error (ExitRequestException) except for remove commands.
   *
   * \throws ExitRequestException In \c non-interactive mode unless \c ignore-unknown
   */
  static void printAndHandleSolverRequesterFeedback( Zypper &zypper, const SolverRequester &sr_r, bool failOnCapNotFound = true );

protected:
  void fillSrOpts (SolverRequester::Options &sropts_r ) const;

  int handlePackageFile (Zypper &zypper, std::vector<std::string> &positionalArgs );

  std::set<zypp::ResKind> _kinds;

  bool _details       = false;
  bool _selectByName  = false;
  bool _selectByCap   = false;
  zypp::Pathname _packageFile;

  InitReposOptionSet _initRepos { *this };
  NoConfirmRugOption _noConfirmOpts { *this };
  DryRunOptionSet _dryRun { *this };
  SolverCommonOptionSet _commonSolverOpts { *this };

  // ZypperBaseCommand interface
protected:
  std::vector<BaseCommandConditionPtr> conditions() const override;
  ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
};

class RemoveCmd : public InstallRemoveBase
{
public:
  RemoveCmd( std::vector<std::string> &&commandAliases_r );

private:
  SolverCleanDepsOptionSet _cleanDeps { *this };

  // ZypperBaseCommand interface
protected:
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r ) override;
};

class InstallCmd : public InstallRemoveBase
{
public:
  InstallCmd( std::vector<std::string> &&commandAliases_r );

  // Alternate form for derived classes like RemovePtfCmd
  InstallCmd( std::vector<std::string> &&commandAliases_r,
              std::string &&synopsis_r,
              std::string &&summary_r = std::string(),
              CommandDescription &&description_r = CommandDescription() );

private:
  bool _force  = false;
  bool _oldPackage = false;
  bool _allowUnsignedRPM = false;
  std::vector<std::string> _fromRepos;
  std::vector<std::string> _entireCatalog;

  FileConflictPolicyOptionSet _fileConflictOpts { *this };
  LicensePolicyOptionSet _licensePolicy { *this };
  DownloadOptionSet _downloadMode { *this };

  SolverRecommendsOptionSet _recommendsSolverOpts { *this };
  SolverInstallsOptionSet _installsSolverOpts { *this };

  // ZypperBaseCommand interface
protected:
  ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r) override;
};

class RemovePtfCmd : public InstallCmd // InstallRemoveBase
{
public:
  RemovePtfCmd( std::vector<std::string> &&commandAliases_r );
protected:
  int execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r ) override;
};

#endif
