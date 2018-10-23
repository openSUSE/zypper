/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_OPTIONSET_H_INCLUDED
#define ZYPPER_COMMANDS_OPTIONSET_H_INCLUDED

/**
 * \file contains all optionsets that do not otherwise belong to a group
 */

#include "basecommand.h"

class DryRunOptionSet : public BaseCommandOptionSet
{
public:
 using BaseCommandOptionSet::BaseCommandOptionSet;

  // BaseCommandOptionSet interface
public:
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;
};

class InitReposOptionSet : public BaseCommandOptionSet
{
public:
 using BaseCommandOptionSet::BaseCommandOptionSet;
  void setEnableRugCompatibility ( bool set = true );

private:
  bool _isRugCmd = false;

  // BaseCommandOptionSet interface
public:
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;
};

#endif
