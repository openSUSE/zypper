/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_SERVICES_LIST_H_INCLUDED
#define ZYPPER_COMMANDS_SERVICES_LIST_H_INCLUDED

#include "commands/basecommand.h"
#include "commands/reposerviceoptionsets.h"
#include "utils/flags/zyppflags.h"

#include <zypp/base/Flags.h>

class ListServicesCmd : public ZypperBaseCommand
{
public:
  ListServicesCmd ( std::vector<std::string> &&commandAliases_r );

private:
  void printServiceList    ( Zypper &zypper );
  void printXMLServiceList ( Zypper &zypper );

  // ZypperBaseCommand interface
protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r) override;

private:
  RSCommonListOptions _listOptions { OptCommandCtx::ServiceContext, *this };
};


#endif
