/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_PATCH_CHECK_INCLUDED
#define ZYPPER_COMMANDS_PATCH_CHECK_INCLUDED

#include "commands/basecommand.h"
#include "commands/optionsets.h"
#include "utils/flags/zyppflags.h"

class PatchCheckCmd : public ZypperBaseCommand
{
public:
  PatchCheckCmd( const std::vector<std::string> &commandAliases_r );

private:
  bool _updateStackOnly = false;
  InitReposOptionSet _initReposOpts { *this };
  OptionalPatchesOptionSet _optionalOpts { *this };

  // ZypperBaseCommand interface
protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r) override;
};

#endif
