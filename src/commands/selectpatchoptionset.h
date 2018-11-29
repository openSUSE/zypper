/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_SELECTPATCHOPTIONSET_INCLUDED
#define ZYPPER_COMMANDS_SELECTPATCHOPTIONSET_INCLUDED

#include "basecommand.h"
#include "issue.h"
#include "src/update.h"

class SelectPatchOptionSet : public BaseCommandOptionSet
{
public:

  enum AnyTypeMode
  {
    EnableAnyType, //< Enables the arguments to take optional args and select any issue type if no arg was given,
                   //  also enables the generic --issue flag
    DisableAnyType
  };

  SelectPatchOptionSet ( ZypperBaseCommand &parent, AnyTypeMode mode_r = DisableAnyType );

  PatchSelector _select;

private:
  AnyTypeMode _mode;

  // BaseCommandOptionSet interface
public:
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;
};

#endif
