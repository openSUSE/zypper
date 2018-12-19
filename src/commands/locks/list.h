/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_LOCKS_LIST_H_INCLUDED
#define ZYPPER_COMMANDS_LOCKS_LIST_H_INCLUDED

#include "commands/basecommand.h"
#include "utils/flags/zyppflags.h"

class ListLocksCmd : public ZypperBaseCommand
{
public:
  ListLocksCmd( std::vector<std::string> &&commandAliases_r );

  // ZypperBaseCommand interface
  std::string description() const override;

protected:
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs) override;
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int systemSetup(Zypper &zypper) override;

private:
  bool _matches   = false;
  bool _solvables = false;
};



#endif
