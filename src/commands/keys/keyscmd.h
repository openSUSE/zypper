/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_KEYS_KEYSCMD_H_INCLUDED
#define ZYPPER_COMMANDS_KEYS_KEYSCMD_H_INCLUDED

#include "commands/basecommand.h"
#include "utils/flags/zyppflags.h"

class KeysCmd : public ZypperBaseCommand
{
public:
  KeysCmd( std::vector<std::string> &&commandAliases_r );

private:
  void init ();
  bool _showDetails;

  // ZypperBaseCommand interface
protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &) override;
};

#endif
