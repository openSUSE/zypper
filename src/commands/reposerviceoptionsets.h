/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_REPO_SERVICE_OPTIONSET_H_INCLUDED
#define ZYPPER_COMMANDS_REPO_SERVICE_OPTIONSET_H_INCLUDED

#include "basecommand.h"

#include <zypp/TriBool.h>

class RepoServiceCommonOptions : public BaseCommandOptionSet
{
public:
  using BaseCommandOptionSet::BaseCommandOptionSet;

  // BaseCommandOptionSet interface
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;

  //will be removed
  void fillFromCopts (Zypper &zypper );

  std::string _name;
  zypp::TriBool _enable = zypp::indeterminate;
  zypp::TriBool _enableAutoRefresh = zypp::indeterminate;
};


#endif
