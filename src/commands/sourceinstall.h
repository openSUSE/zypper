/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_SOURCEINSTALL_INCLUDED
#define ZYPPER_COMMANDS_SOURCEINSTALL_INCLUDED

#include "commands/basecommand.h"
#include "commands/optionsets.h"
#include "commands/solveroptionset.h"
#include "utils/flags/zyppflags.h"

class SourceInstallCmd : public ZypperBaseCommand
{
public:
  SourceInstallCmd( std::vector<std::string> &&commandAliases_r );

private:
  bool _buildDepsOnly = false;
  bool _noBuildDeps   = false;
  InitReposOptionSet _initRepos { *this };
  DownloadOptionSet _dlOpts { *this, DownloadOptionSet::SourceInstall };
  SolverCommonOptionSet _solverCommonOpts { *this };
  SolverInstallsOptionSet _solverInstallOpts { *this };
  SolverRecommendsOptionSet _solverRecommOpts { *this };

  // ZypperBaseCommand interface
protected:
  std::vector<BaseCommandConditionPtr> conditions() const override;
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r) override;
};

#endif
