/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_SERVICES_REFRESH_H_INCLUDED
#define ZYPPER_COMMANDS_SERVICES_REFRESH_H_INCLUDED

#include "commands/basecommand.h"
#include "utils/flags/zyppflags.h"

#include <zypp/base/Flags.h>

class RefreshServicesCmd : public ZypperBaseCommand
{

public:
  RefreshServicesCmd ( bool force_r = false, bool withRepos_r = false, bool restoreStatus_r = false );

  int refreshServices ( Zypper &zypper );

  // ZypperBaseCommand interface
protected:
  std::vector<BaseCommandConditionPtr> conditions() const override;
  std::vector<zypp::ZyppFlags::CommandOption> cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs) override;
  int systemSetup(Zypper &zypp_r) override;

private:
  bool _force;
  bool _withRepos;
  bool _restoreStatus;
};

#endif
