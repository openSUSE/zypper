/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_CONFIGTEST_H
#define ZYPPER_CONFIGTEST_H

#include "commands/basecommand.h"
#include "commands/optionsets.h"
#include "utils/flags/zyppflags.h"

class ConfigTestCmd : public ZypperBaseCommand
{
public:
  ConfigTestCmd ( std::vector<std::string> &&commandAliases_r );

  // ZypperBaseCommand interface
protected:
  ZyppFlags::CommandGroup cmdOptions() const override { return {}; }
  void doReset() override {}
  int execute(Zypper &zypper, const std::vector<std::string> &) override;
};

#endif // ZYPPER_CONFIGTEST_H
