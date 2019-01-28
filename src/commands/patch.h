/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_PATCH_INCLUDED
#define ZYPPER_COMMANDS_PATCH_INCLUDED

#include "commands/basecommand.h"
#include "utils/flags/zyppflags.h"
#include "selectpatchoptionset.h"
#include "optionsets.h"
#include "solveroptionset.h"
#include "issue.h"

#include <zypp/Patch.h>

class PatchCmd : public ZypperBaseCommand
{
public:
  PatchCmd( std::vector<std::string> &&commandAliases_r );

private:
  bool _updateStackOnly = false;
  bool _withUpdate = false;
  bool _details = false;

  FileConflictPolicyOptionSet _fileConflictOpts { *this };
  InitReposOptionSet _initRepoOpts { *this };
  SelectPatchOptionSet _selectPatchOpts { *this };
  NoConfirmRugOption _noConfirmOpts { *this };
  OptionalPatchesOptionSet _optionalPatchesOpts { *this };
  InteractiveUpdatesOptionSet _interactiveUpdatesOpts { *this };
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
