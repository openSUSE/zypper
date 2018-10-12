/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_REPOS_CLEAN_H_INCLUDED
#define ZYPPER_COMMANDS_REPOS_CLEAN_H_INCLUDED

#include "commands/basecommand.h"
#include "repos.h"

#include <string>
#include <vector>

class CleanRepoCmd : public ZypperBaseCommand
{
public:
  CleanRepoCmd( const std::vector<std::string> &commandAliases_r );

  // ZypperBaseCommand interface
protected:
  std::vector<BaseCommandConditionPtr> conditions() const override;
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r) override;

private:
  std::vector<std::string> _repos;
  CleanRepoFlags _flags;
};

#endif
