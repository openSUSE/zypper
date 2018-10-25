/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_QUERY_PATTERNS_INCLUDED
#define ZYPPER_COMMANDS_QUERY_PATTERNS_INCLUDED

#include "commands/basecommand.h"
#include "commands/optionsets.h"
#include "utils/flags/zyppflags.h"

#include "src/search.h"
#include "miscqueryinit.h"

class PatternsCmdBase : public ZypperBaseCommand
{
public:
  PatternsCmdBase( const std::vector<std::string> &commandAliases_r );

private:
  InitReposOptionSet _initRepoFlags { *this };
  NotInstalledOnlyOptionSet _instOnlyFlags { *this };

  // ZypperBaseCommand interface
protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;

  int execute(Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r) override;
};

using PatternsCmd = MiscQueryInitMixin<PatternsCmdBase>;

#endif
