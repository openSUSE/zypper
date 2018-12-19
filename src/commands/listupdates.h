/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_LISTUPDATES_INCLUDED
#define ZYPPER_COMMANDS_LISTUPDATES_INCLUDED

#include "commands/basecommand.h"
#include "utils/flags/zyppflags.h"
#include "optionsets.h"

#include <zypp/ResKind.h>

class ListUpdatesCmd : public ZypperBaseCommand
{
public:
  ListUpdatesCmd( std::vector<std::string> &&commandAliases_r );

private:
  std::set<ResKind> _kinds;
  bool _all = false;
  bool _bestEffort = false;
  InitReposOptionSet _initReposOpts { *this };


  // ZypperBaseCommand interface
protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r) override;
};

#endif
