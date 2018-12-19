/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_UPDATE_INCLUDED
#define ZYPPER_COMMANDS_UPDATE_INCLUDED

#include "commands/basecommand.h"
#include "commands/optionsets.h"
#include "commands/solveroptionset.h"
#include "utils/flags/zyppflags.h"

#include <zypp/ResKind.h>

class UpdateCmd : public ZypperBaseCommand
{
public:
  UpdateCmd( std::vector<std::string> &&commandAliases_r );

private:
  bool _details = false;
  bool _replaceFiles = false;
  bool _bestEffort   = false;
  std::set<zypp::ResKind> _kinds;
  InitReposOptionSet _initReposOpts { *this };
  NoConfirmRugOption _noComfirmOpts { *this };
  InteractiveUpdatesOptionSet _interactiveOpts { *this };
  LicensePolicyOptionSet _licensePolicyOpts { *this };
  DryRunOptionSet _dryRunOpts { *this };
  DownloadOptionSet _downloadModeOpts { *this };
  SolverCommonOptionSet _commonSolverOpts { *this };
  SolverRecommendsOptionSet _recommendsSolverOpts { *this };
  SolverInstallsOptionSet _installSolverOpts { *this };


  // ZypperBaseCommand interface
protected:
  std::vector<BaseCommandConditionPtr> conditions() const override;
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r) override;
};

#endif
