/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_UTILS_PURGEKERNELS_INCLUDED
#define ZYPPER_COMMANDS_UTILS_PURGEKERNELS_INCLUDED

#include "commands/basecommand.h"
#include "commands/optionsets.h"
#include "utils/flags/zyppflags.h"

class PurgeKernelsCmd : public ZypperBaseCommand
{
public:
  PurgeKernelsCmd( std::vector<std::string> &&commandAliases_r );
  // ZypperBaseCommand interface

protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &) override;
  std::vector<BaseCommandConditionPtr> conditions() const override;

private:
  DryRunOptionSet _dryRun { *this };
  bool _details = false;
};

#endif
