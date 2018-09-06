/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_CONDITIONS_H_INCLUDED
#define ZYPPER_COMMANDS_CONDITIONS_H_INCLUDED

#include "basecommand.h"

class NeedsRootCondition : public BaseCommandCondition
{
public:
  // BaseCommandCondition interface
  int check(std::string &err) override;
};

#endif
