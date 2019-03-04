/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_INSTALLREMOVE_INCLUDED
#define ZYPPER_COMMANDS_INSTALLREMOVE_INCLUDED

#include "commands/basecommand.h"
#include "utils/flags/zyppflags.h"
#include "commands/optionsets.h"
#include "commands/solveroptionset.h"
#include "SolverRequester.h"

#include <zypp/ResObject.h>

class InstallRemoveBase : public ZypperBaseCommand
{
public:
  using ZypperBaseCommand::ZypperBaseCommand;
protected:
  void fillSrOpts (SolverRequester::Options &sropts_r ) const;
  void handleFeedback(Zypper &zypper, const SolverRequester &sr_r, bool failOnCapNotFound = true ) const;

  std::set<zypp::ResKind> _kinds;

  bool _details       = false;
  bool _selectByName  = false;
  bool _selectByCap   = false;

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
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs) override;
};

class InstallCmd : public InstallRemoveBase
{
public:
  InstallCmd( std::vector<std::string> &&commandAliases_r );

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

#endif
