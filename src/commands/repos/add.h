/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_REPOS_ADD_H_INCLUDED
#define ZYPPER_COMMANDS_REPOS_ADD_H_INCLUDED

#include "commands/basecommand.h"
#include "commands/reposerviceoptionsets.h"

#include <zypp/TriBool.h>

#include <string>

class AddRepoCmd : public ZypperBaseCommand
{
public:
  AddRepoCmd();

  // ZypperBaseCommand interface
protected:
  std::vector<BaseCommandConditionPtr> conditions() const override;
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r) override;

private:
  RepoProperties _repoProperties{*this};
  RepoServiceCommonOptions _commonProperties{OptCommandCtx::RepoContext, *this};
  std::string _repoFile;
  bool        _enableCheck  = false;
  bool        _disableCheck = false;
};

#endif
