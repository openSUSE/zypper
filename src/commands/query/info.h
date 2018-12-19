/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_QUERY_INFO_INCLUDED
#define ZYPPER_COMMANDS_QUERY_INFO_INCLUDED

#include "commands/basecommand.h"
#include "commands/optionsets.h"
#include "utils/flags/zyppflags.h"
#include "src/info.h"

#include <zypp/ResKind.h>
#include <zypp/base/Flags.h>

#include <set>

class InfoCmd : public ZypperBaseCommand
{
public:

  enum class Mode {
    Default,
    RugPatchInfo,
    RugPatternInfo,
    RugProductInfo
  };

  InfoCmd ( std::vector<std::string> &&commandAliases_r, Mode cmdMode_r = Mode::Default );

  // ZypperBaseCommand interface
public:
  std::string help() override;
  std::string summary() const override;
  std::vector<std::string> synopsis() const override;
  std::string description() const override;

protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r) override;

private:
  Mode _cmdMode = Mode::Default;
  PrintInfoOptions _options;
  InitReposOptionSet _initRepoOpts { *this };
};

#endif
