/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_COMMANDS_UTILS_CONFIGCHECK_INCLUDED
#define ZYPPER_COMMANDS_UTILS_CONFIGCHECK_INCLUDED

#include <vector>
#include <string>

#include "commands/basecommand.h"
#include "commands/optionsets.h"
#include "utils/flags/zyppflags.h"

// Component extracted for unit testing
struct ConfigCheck
{
  std::vector<std::string> getSystemConfigFiles() const;
  
  // Scans for .rpmnew and .rpmsave given a list of base config files.
  // Missing files are ignored. Permission errors emit a warning to Zypper output (or can be tracked).
  // Returns true if successfully completed (even if finding files).
  bool run(Zypper &zypper, const std::vector<std::string> &configFiles);
};

class ConfigCheckCmd : public ZypperBaseCommand
{
public:
  ConfigCheckCmd( std::vector<std::string> &&commandAliases_r );

protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &) override;
  std::vector<BaseCommandConditionPtr> conditions() const override;
};

#endif
