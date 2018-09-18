/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_SERVICES_ADD_H_INCLUDED
#define ZYPPER_COMMANDS_SERVICES_ADD_H_INCLUDED

#include "commands/basecommand.h"
#include "commands/reposerviceoptionsets.h"
#include "utils/flags/zyppflags.h"

#include <zypp/base/Flags.h>
#include <zypp/repo/ServiceType.h>

class AddServiceCmd : public ZypperBaseCommand
{
public:
  AddServiceCmd();

  // ZypperBaseCommand interface
protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute( Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r ) override;
  std::vector<BaseCommandConditionPtr> conditions() const override;

private:
  bool _isService = true;
  RepoServiceCommonOptions _commonProps{*this};
};

#endif
