/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_HELP_INCLUDED
#define ZYPPER_COMMANDS_HELP_INCLUDED

#include "commands/basecommand.h"
#include "utils/flags/zyppflags.h"

class HelpCmd : public ZypperBaseCommand
{
public:
  HelpCmd ( std::vector<std::string> &&commandAliases_r );

  static void printMainHelp(Zypper &zypper);

  // ZypperBaseCommand interface
protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &) override;
};

#endif
