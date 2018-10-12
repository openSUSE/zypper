/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_LOCKS_ADD_H_INCLUDED
#define ZYPPER_COMMANDS_LOCKS_ADD_H_INCLUDED

#include "commands/basecommand.h"
#include "utils/flags/zyppflags.h"

#include <zypp/ResKind.h>
#include <set>

class AddLocksCmd : public ZypperBaseCommand
{
public:
  AddLocksCmd( const std::vector<std::string> &commandAliases_r );

protected:
  // ZypperBaseCommand interface
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r) override;
  std::vector<BaseCommandConditionPtr> conditions() const override;

private:
  std::set<zypp::ResKind> _kinds;
  std::vector<std::string> _repos;
};



#endif
