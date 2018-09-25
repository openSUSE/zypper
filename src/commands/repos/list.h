/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_REPOS_LIST_H_INCLUDED
#define ZYPPER_COMMANDS_REPOS_LIST_H_INCLUDED

#include "commands/basecommand.h"
#include "commands/reposerviceoptionsets.h"
#include "utils/flags/zyppflags.h"

#include <zypp/base/Flags.h>
#include <zypp/RepoInfo.h>

class ListReposCmd : public ZypperBaseCommand
{
public:
  ListReposCmd();

  // ZypperBaseCommand interface
protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r) override;
  void printRepoList(Zypper &zypper, const std::list<zypp::RepoInfo> &repos);

private:
  std::string _exportFile;
  RSCommonListOptions _listOptions{ OptCommandCtx::RepoContext, *this };
};

#endif
