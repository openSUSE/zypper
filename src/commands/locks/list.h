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
  // ZypperBaseCommand interface
public:
  std::list<std::string> command() const override;
  std::string summary() const override;
  std::string synopsis() const override;
  std::string description() const override;
  LoadSystemFlags needSystemSetup() const override;

protected:
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs) override;
  std::vector<zypp::ZyppFlags::CommandOption> cmdOptions() const override;
  void doReset() override;

private:
  bool _matches   = false;
  bool _solvables = false;
};



#endif
