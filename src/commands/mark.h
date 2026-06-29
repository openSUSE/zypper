/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_MARKAUTO_H
#define ZYPPER_MARKAUTO_H

#include <string>
#include <zypp/ResKind.h>

#include "commands/basecommand.h"
#include "commands/optionsets.h"
#include "commands/conditions.h"

class Zypper;

class MarkCmd : public ZypperBaseCommand
{
public:
  MarkCmd ( std::vector<std::string> &&commandAliases_r );

private:
  void init ();
  zypp::ResKind _kind;
  bool _selectByName;
  bool _selectByCap;

  DryRunOptionSet _dryRunOpts { *this };

  // ZypperBaseCommand interface
protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute( Zypper &zypper, const std::vector<std::string> &positionalArgs ) override;
  std::vector<BaseCommandConditionPtr> conditions() const override;
};

#endif // ZYPPER_MARKAUTO_H
