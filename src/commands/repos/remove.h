/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_REPOS_REMOVE_H_INCLUDED
#define ZYPPER_COMMANDS_REPOS_REMOVE_H_INCLUDED

#include "commands/basecommand.h"
#include "commands/reposerviceoptionsets.h"

#include <zypp/TriBool.h>

#include <string>

class RemoveRepoCmd : public ZypperBaseCommand
{
public:
  RemoveRepoCmd( const std::vector<std::string> &commandAliases_r );

  // ZypperBaseCommand interface
protected:
  std::vector<BaseCommandConditionPtr> conditions() const override;
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypp, const std::vector<std::string> &positionalArgs) override;

private:
  bool _looseAuth = false;
  bool _looseQuery = false;
  RepoServiceCommonSelectOptions _selections{OptCommandCtx::RepoContext, *this};
};

#endif
