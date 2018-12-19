/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_UTILS_TARGETOS_INCLUDED
#define ZYPPER_COMMANDS_UTILS_TARGETOS_INCLUDED

#include "commands/basecommand.h"
#include "utils/flags/zyppflags.h"

class TargetOSCmd : public ZypperBaseCommand
{
public:
  TargetOSCmd( std::vector<std::string> &&commandAliases_r );
  // ZypperBaseCommand interface
protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &) override;

private:
  bool _showOSLabel = false;
};

#endif
