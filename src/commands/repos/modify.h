/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_REPOS_MODIFY_H_INCLUDED
#define ZYPPER_COMMANDS_REPOS_MODIFY_H_INCLUDED

#include "commands/basecommand.h"
#include "commands/reposerviceoptionsets.h"

class ModifyRepoCmd : public ZypperBaseCommand
{
public:
  ModifyRepoCmd();

  // ZypperBaseCommand interface
  std::string help() override;

protected:
  std::vector<BaseCommandConditionPtr> conditions() const override;
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r) override;

private:
  RepoServiceCommonOptions _commonProps{OptCommandCtx::RepoContext, *this};
  RepoProperties _repoProps{*this};
  RepoServiceCommonSelectOptions _selections{OptCommandCtx::RepoContext, *this};
};

#endif
