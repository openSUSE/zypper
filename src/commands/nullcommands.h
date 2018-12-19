/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_NULLCOMMANDS_INCLUDED
#define ZYPPER_COMMANDS_NULLCOMMANDS_INCLUDED

#include "commands/basecommand.h"
#include "commands/optionsets.h"
#include "utils/flags/zyppflags.h"

class MooCmd : public ZypperBaseCommand
{
public:
  MooCmd ( const std::vector<std::string> &commandAliases_r );

  // ZypperBaseCommand interface
protected:
  ZyppFlags::CommandGroup cmdOptions() const override { return {}; }
  void doReset() override {}
  int execute(Zypper &zypper, const std::vector<std::string> &) override;
};

class WhatProvidesCmd : public ZypperBaseCommand
{
public:
  WhatProvidesCmd ( const std::vector<std::string> &commandAliases_r );


  // ZypperBaseCommand interface
protected:
  ZyppFlags::CommandGroup cmdOptions() const override { return {}; }
  void doReset() override {}
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r) override;
};

class RupPingCmd : public ZypperBaseCommand
{
public:
  RupPingCmd ( const std::vector<std::string> &commandAliases_r );

  // ZypperBaseCommand interface
protected:
  ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override {}
  int execute(Zypper &, const std::vector<std::string> &) override;
};

#endif
