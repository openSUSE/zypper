/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_INRVERIFY_INCLUDED
#define ZYPPER_COMMANDS_INRVERIFY_INCLUDED

#include "commands/basecommand.h"
#include "commands/optionsets.h"
#include "commands/solveroptionset.h"
#include "utils/flags/zyppflags.h"

/**
 * Combines the verify and install-new-recommends command, if changing the behaviour
 * of one of them consider to split them in 2 different commands.
 */
class InrVerifyCmd : public ZypperBaseCommand
{
public:

  enum Mode {
    InstallRecommends,
    Verify
  };

  InrVerifyCmd( const std::vector<std::string> &commandAliases_r, Mode cmdMode_r );

private:
  Mode _mode = InstallRecommends;
  bool _details = false;
  NoConfirmRugOption _noConfirmOpts; //< not registered by default since only verify offers it
  DryRunOptionSet _dryRunOpts { *this };
  DownloadOptionSet _downloadOpts { *this };
  InitReposOptionSet _initRepoOpts { *this };
  SolverCommonOptionSet _commonSolverOpts { *this };
  SolverRecommendsOptionSet _recommendsSolverOpts { *this };
  SolverInstallsOptionSet _installSolverOpts { *this };

  // ZypperBaseCommand interface
public:
  std::string summary() const override;
  std::vector<std::string> synopsis() const override;
  std::string description() const override;

protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r) override;
  std::vector<BaseCommandConditionPtr> conditions() const override;
};

#endif
