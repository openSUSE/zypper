/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_SERVICES_LIST_H_INCLUDED
#define ZYPPER_COMMANDS_SERVICES_LIST_H_INCLUDED

#include "commands/basecommand.h"
#include "utils/flags/zyppflags.h"

#include <zypp/base/Flags.h>

class ListServicesCmd : public ZypperBaseCommand
{
public:
  enum ServiceListFlagsBits
  {
    ShowAll            = 7,
    ShowURI            = 1,
    ShowPriority       = 1 << 1,
    ShowWithRepos      = 1 << 2,
    ShowEnabledOnly    = 1 << 3,
    SortByURI          = 1 << 4,
    SortByName         = 1 << 5,
    SortByPrio         = 1 << 6,
    ServiceRepo        = 1 << 15
  };
  ZYPP_DECLARE_FLAGS( ServiceListFlags,ServiceListFlagsBits );

  ListServicesCmd ();

private:
  void printServiceList    ( Zypper &zypp_r );
  void printXMLServiceList ( Zypper &zypp_r );

  // ZypperBaseCommand interface
protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r) override;

private:
  ServiceListFlags _flags;
};
ZYPP_DECLARE_OPERATORS_FOR_FLAGS( ListServicesCmd::ServiceListFlags )


#endif
