/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_SERVICES_MODIFY_H_INCLUDED
#define ZYPPER_COMMANDS_SERVICES_MODIFY_H_INCLUDED

#include "commands/basecommand.h"
#include "commands/reposerviceoptionsets.h"
#include "utils/flags/zyppflags.h"

#include <zypp/base/Flags.h>

class ModifyServiceCmd : public ZypperBaseCommand
{
public:
  ModifyServiceCmd( std::vector<std::string> &&commandAliases_r );

  // ZypperBaseCommand interface
  std::string help() override;

protected:
  std::vector<BaseCommandConditionPtr> conditions() const override;
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r) override;

private:
  int modifyService( Zypper & zypper, const std::string & alias );
  void modifyServicesByOption( Zypper & zypper );

private:
  std::vector<std::string> _arToEnable;
  std::vector<std::string> _arToDisable;
  std::vector<std::string> _rrToEnable;
  std::vector<std::string> _rrToDisable;
  bool _clearToEnable  = false;
  bool _clearToDisable = false;
  RepoServiceCommonOptions _commonProperties { OptCommandCtx::ServiceContext, *this };
  RepoServiceCommonSelectOptions _selectOptions { OptCommandCtx::ServiceContext, *this };
};

#endif
