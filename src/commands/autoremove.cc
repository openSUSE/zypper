/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include <zypp/ZYpp.h> // for ResPool::instance()
#include "autoremove.h"
#include "utils/flags/flagtypes.h"
#include "commands/conditions.h"
#include "solve-commit.h"
#include "utils/messages.h"
#include "src/SolverRequester.h"
#include "commonflags.h"
#include "Zypper.h"

extern ZYpp::Ptr God;

AutoRemoveCmd::AutoRemoveCmd(std::vector<std::string> &&commandAliases_r) : ZypperBaseCommand(std::move(commandAliases_r),
                                                                                              // translators: command synopsis; do not translate lowercase words
                                                                                              _("autoremove (arm) [OPTIONS]"),
                                                                                              // translators: command summary: dist-upgrade, dup
                                                                                              _("Remove unneeded packages due to updates or installs."), // TODO: better explnations
                                                                                              // translators: command description
                                                                                              _("Remove a unneeded packages."),
                                                                                              ResetRepoManager | InitTarget | InitRepos | LoadResolvables)
{
    _dryRunOpts.setCompatibilityMode(CompatModeBits::EnableNewOpt | CompatModeBits::EnableRugOpt);
}

zypp::ZyppFlags::CommandGroup AutoRemoveCmd::cmdOptions() const
{
    auto that = const_cast<AutoRemoveCmd *>(this);
    return zypp::ZyppFlags::CommandGroup({{"remove-orphaned", '\0',
                                           zypp::ZyppFlags::NoArgument,
                                           zypp::ZyppFlags::BoolType(&that->_orpahned, ZyppFlags::StoreTrue, _orpahned),
                                           _("Remove unneeded orphaned packages.")},
                                          CommonFlags::detailsFlag(that->_details)});
}

void AutoRemoveCmd::doReset()
{
    ArmSettings::reset();
    _details = false;
}

std::vector<BaseCommandConditionPtr> AutoRemoveCmd::conditions() const
{
    return {
        std::make_shared<NeedsRootCondition>(),
        std::make_shared<NeedsWritableRoot>()};
}

int AutoRemoveCmd::execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r)
{
    // too many arguments
    if (positionalArgs_r.size() > 0)
    {
        report_too_many_arguments(help());
        return (ZYPPER_EXIT_ERR_INVALID_ARGS);
    }

    int code = defaultSystemSetup(zypper, InitTarget | InitRepos | LoadResolvables | Resolve);
    if (code != ZYPPER_EXIT_OK)
        return code;

    zypper.out().warning(str::form(
        _("You are run autoremove on the system. This is experimental and subject to change"
          " This may remove unintended packages please confirm the packages that will be removed before you"
          " continue. See '%s' for more information about this command."),
        "man zypper"));

    std::vector<std::string> pkgs;

    auto checkStatus = [=](const PoolItem &pi_r) -> bool
    {
        // isipi : prevent returning true if identical installed items are tested.
        const ResStatus &status{pi_r.status()};
        if ((_orpahned && status.isOrphaned()) || status.isUnneeded())
        {
            return true;
        }
        return false;
    };

    God->resolver()->resolvePool();

    SolverRequester::Options sropts;

    SolverRequester sr(sropts);

    for (const auto &sel : God->pool().proxy().byKind<Package>())
    {
        for (const auto &pi : sel->picklist())
        {

            if (pi.status().isInstalled())
            {
                if (!checkStatus(pi))
                    continue;
            }
            else
            {
                PoolItem ipi(sel->identicalInstalledObj(pi));
                if (!ipi || !checkStatus(ipi))
                    if (!checkStatus(pi))
                        continue;
            }
            std::vector<std::string> a;
            a.push_back(pi.asString());
            sr.remove(a);
        }
    }

    Summary::ViewOptions opts = Summary::DEFAULT;
    if (_details)
        opts = static_cast<Summary::ViewOptions>(opts | Summary::DETAILS);
    // do solve
    solve_and_commit(zypper, SolveAndCommitPolicy().summaryOptions(opts));
    return zypper.exitCode();
}