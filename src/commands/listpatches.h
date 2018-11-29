/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_LISTPATCHES_INCLUDED
#define ZYPPER_COMMANDS_LISTPATCHES_INCLUDED

#include "commands/basecommand.h"
#include "commands/selectpatchoptionset.h"
#include "utils/flags/zyppflags.h"
#include "optionsets.h"

class ListPatchesCmd : public ZypperBaseCommand
{
public:
  ListPatchesCmd( const std::vector<std::string> &commandAliases_r );

private:
  bool _all = false;
  InitReposOptionSet _initReposOpts { *this };
  SelectPatchOptionSet _selectPatchOpts { *this, SelectPatchOptionSet::EnableAnyType };
  OptionalPatchesOptionSet _optionalPatchesOpts { *this };


  // ZypperBaseCommand interface
protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r) override;
};

#endif
