/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_AUTOREMOVE_INCLUDED
#define ZYPPER_COMMANDS_AUTOREMOVE_INCLUDED

#include "commands/basecommand.h"
#include "commands/optionsets.h"
#include "commands/solveroptionset.h"

class AutoRemoveCmd : public ZypperBaseCommand
{
public:
    AutoRemoveCmd(std::vector<std::string> &&commandAliases_r);

private:
    bool _details = false;
    bool _orphaned = false;
    DryRunOptionSet _dryRunOpts{*this};
    NoConfirmRugOption _noConfirmOpts{*this};
    DownloadOptionSet _downloadModeOpts{*this};
    SolverCommonOptionSet _commonSolverOpts{*this};
    SolverRecommendsOptionSet _recommendsSolverOpts{*this};
    SolverInstallsOptionSet _installSolverOpts{*this};

    // ZypperBaseCommand interface
protected:
    zypp::ZyppFlags::CommandGroup cmdOptions() const override;
    void doReset() override;
    int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r) override;
    std::vector<BaseCommandConditionPtr> conditions() const override;
};

#endif