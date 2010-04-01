/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/


/** \file SolverRequester.cc
 * 
 */

#include "zypp/ZYppFactory.h"
#include "zypp/base/LogTools.h"

#include "zypp/PoolQuery.h"
#include "zypp/PoolItemBest.h"

#include "zypp/Capability.h"
#include "zypp/Resolver.h"
#include "zypp/Patch.h"
#include "zypp/ui/Selectable.h"

#include "Zypper.h" // temporarily!!!
#include "misc.h"

#include "SolverRequester.h"

using namespace std;
using namespace zypp;
using namespace zypp::ui;

void SolverRequester::install(const PackageArgs & args)
{ installRemove(args, true); }

void SolverRequester::remove(const PackageArgs & args)
{ installRemove(args, true); }

void SolverRequester::installRemove(const PackageArgs & args, bool doinst)
{
  if (args.empty())
    return;

  for_(it, args.doCaps().begin(), args.doCaps().end())
    install(it->first, it->second);

  for_(it, args.dontCaps().begin(), args.dontCaps().end())
    remove(it->first);
}

// ----------------------------------------------------------------------------

/**
 * For given Capability & repo & Options:
 *
 * 1) if the --capability was not specified, try to install 'by name' first.
 *    I.e. via ui::Selectable and/or PoolItem.
 *    note: wildcards must be supported here, use PoolQuery with match_glob
 *          to find the selectables
 *
 * 2) if no package could be found by name and --name was not specified,
 *    or --capability was specified, install 'by capability',
 *    i.e. using ResPool::addRequires(cap)
 *
 * NOTES
 * - If the argument contains repository, issue request forcing the repo
 * - In both cases check for already installed packages, and if found, hand over
 *   to the \ref update(const Capability&, const string&) method.
 *
 * TODO
 * - maybe a check for glob wildcards in cap name would make sense before trying
 *   by-cap
 */
void SolverRequester::install(const Capability & cap, const string & repoalias)
{
#warning get rid of zypper here
  Zypper & zypper = *Zypper::instance();

  sat::Solvable::SplitIdent splid(cap.detail().name());
  ResKind capkind = splid.kind();
  string capname = splid.name().asString();

  // first try by name

  if (!_opts.force_by_cap)
  {
    PoolQuery q = pkg_spec_to_poolquery(cap, _opts.from_repos);
    if (!repoalias.empty())
      q.addRepo(repoalias);

    // get the best matching items and tag them for installation.
    PoolItemBest bestMatches(q.begin(), q.end());
    if (!bestMatches.empty())
    {
      for_(sit, bestMatches.begin(), bestMatches.end())
      {
        if (asSelectable()(*sit)->hasInstalledObj())
          update(cap, repoalias);
        else
          asSelectable()(*sit)->setOnSystem(*sit, ResStatus::USER);
        // TODO handle patches (isBroken), patterns (isSatisfied instead of hasInstalledObj)
      }
    }
    else if (_opts.force_by_name)
    {
      //! \todo report this via error class
      // translators: meaning a package %s or provider of capability %s
      zypper.out().error(str::form(_("'%s' not found."), capname.c_str()));
      WAR << str::form("'%s' not found", capname.c_str()) << endl;
      zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
      if (zypper.globalOpts().non_interactive)
        ZYPP_THROW(ExitRequestException());
    }
  }

  if (_opts.force_by_name)
    return;

  // try by capability
  // TODO tell that we're falling back to search by capability

  // is there a provider for the requested capability?
  sat::WhatProvides q(cap);
  if (q.empty())
  {
    // translators: meaning a package %s or provider of capability %s
    zypper.out().error(str::form(_("'%s' not found."), cap.asString().c_str()));
    WAR << str::form("'%s' not found", cap.asString().c_str()) << endl;
    zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
    if (zypper.globalOpts().non_interactive)
      ZYPP_THROW(ExitRequestException());
    return;
  }

  // is the provider already installed?
  set<PoolItem> providers = get_installed_providers(cap);
  // already installed, try to update()
  for_(it, providers.begin(), providers.end())
  {
    if (it->resolvable()->name() == capname)
      zypper.out().info(
          str::form(_("'%s' is already installed."), capname.c_str()));
    else
      zypper.out().info(str::form(
          // translators: %s are package names
          _("'%s' providing '%s' is already installed."),
          (*it)->name().c_str(), cap.asString().c_str()));

    MIL << "provider '" << *it << "' of '" << cap << "' already installed;"
        << " will try to update" << endl;

    Capability pcap((*it)->name(), (*it)->kind());
    update(pcap, repoalias);
  }

  if (providers.empty())
  {
    DBG << "adding requirement " << cap << endl;
    Resolver_Ptr resolver = zypp::getZYpp()->resolver();
    resolver->addRequire(cap);
  }
}

// ----------------------------------------------------------------------------

/**
 * Remove packages based on given Capability & Options from the system.
 */
void SolverRequester::remove(const Capability & cap)
{
#warning get rid of zypper here
  Zypper & zypper = *Zypper::instance();

  sat::Solvable::SplitIdent splid(cap.detail().name());
  ResKind capkind = splid.kind();
  string capname = splid.name().asString();

  // first try by name

  if (!_opts.force_by_cap)
  {
    PoolQuery q = pkg_spec_to_poolquery(cap, "");
    q.setInstalledOnly();

    if (!q.empty())
    {
      for_(it, q.poolItemBegin(), q.poolItemEnd())
      {
        DBG << "Marking for deletion: " << *it << endl;
        it->status().setToBeUninstalled(ResStatus::USER);
      }
      // TODO handle patches (cannot uninstall!), patterns (remove content(?))
    }
    else if (_opts.force_by_name)
    {
      //! \todo report this via error class
      // translators: meaning a package %s or provider of capability %s
      zypper.out().error(str::form(_("'%s' not found."), capname.c_str()));
      WAR << str::form("'%s' not found", capname.c_str()) << endl;
      zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
      if (zypper.globalOpts().non_interactive)
        ZYPP_THROW(ExitRequestException());
    }
  }

  if (_opts.force_by_name)
    return;

  // try by capability
  // TODO tell that we're falling back to search by capability

  // is there a provider for the requested capability?
  sat::WhatProvides q(cap);
  if (q.empty())
  {
    // translators: meaning a package %s or provider of capability %s
    zypper.out().error(str::form(_("'%s' not found."), cap.asString().c_str()));
    WAR << str::form("'%s' not found", cap.asString().c_str()) << endl;
    zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
    if (zypper.globalOpts().non_interactive)
      ZYPP_THROW(ExitRequestException());
    return;
  }

  // is the provider already installed?
  set<PoolItem> providers = get_installed_providers(cap);

  // not installed, nothing to do
  if (providers.empty())
  {
    // translators: meaning a package %s or provider of capability %s
    zypper.out().info(str::form(_("No provider of '%s' is installed."), cap.asString().c_str()));
    MIL << "no provider of " << cap << "is installed" << endl;
  }
  else
  {
    MIL << "adding conflict " << cap << endl;
    Resolver_Ptr resolver = zypp::getZYpp()->resolver();
    resolver->addConflict(cap);
  }
}

// ----------------------------------------------------------------------------

void SolverRequester::update(const PackageArgs & args)
{
  if (args.empty())
    return;

  for_(it, args.doCaps().begin(), args.doCaps().end())
    update(it->first, it->second);

  /* TODO Solve and unmark dont which are setToBeInstalled in the pool?
  for_(it, args.dontCaps().begin(), args.dontCaps().end())
    remove(it->first);
  */
}

// ----------------------------------------------------------------------------

/*
 * Asserting that at least one provider of given \a cap is already installed,
 * this method checks for available update candidates and tries to select
 * the best for installation (thus update). Reports any problems or interesting
 * info back to user.
 *
 * NOTES
 * - If the argument contains repository, issue request forcing the repo and
 *   - if --force was specified, insist on installing from that repo (even
 *     downgrade or change vendor or low priority) TODO
 *   - if --force was NOT used, only upgrade without vendor change or priority
 *     violation. If downgrade or vendor change is needed to get the highest
 *     version, report back to user and advice to use --force,
 *     if that's what is really wanted. TODO
 */
void SolverRequester::update(const Capability & cap, const string & repoalias)
{
  // TODO report 'newest already installed' (see mark_by_name)
}

// ----------------------------------------------------------------------------

void SolverRequester::updatePatterns()
{

}

// ----------------------------------------------------------------------------

void SolverRequester::updatePatches()
{
  DBG << "going to mark needed patches for installation" << endl;

  // search twice: if there are none with restartSuggested(), retry on all
  // (in the first run, ignore_pkgmgmt == 0, in the second it is 1)
  bool any_marked = false;
  for (unsigned ignore_pkgmgmt = 0;
       !any_marked && ignore_pkgmgmt < 2; ++ignore_pkgmgmt)
  {
    for_(it, zypp::getZYpp()->pool().proxy().byKindBegin(ResKind::patch),
             zypp::getZYpp()->pool().proxy().byKindEnd  (ResKind::patch))
    {
      if (installPatch(**it, ignore_pkgmgmt))
        any_marked = true;
    }

    if (any_marked && !ignore_pkgmgmt)
      MIL << "got some pkgmgmt patches, will install these first" << endl;
  }
}

// ----------------------------------------------------------------------------

bool SolverRequester::installPatch(Selectable & s, bool ignore_pkgmgmt)
{
#warning get rid of zypper here
  Zypper & zypper = *Zypper::instance();

  Patch::constPtr patch = asKind<Patch>(s.candidateObj());
  if (s.isBroken()) // bnc #506860
  {
    DBG << "broken candidate patch " << patch
        << " affects_pkgmgmt: " << patch->restartSuggested()
        << (ignore_pkgmgmt ? " (ignored)" : "") << endl;

    if (ignore_pkgmgmt || patch->restartSuggested())
    {
      // bnc #221476
      if (_opts.skip_interactive
          && (patch->interactive() || !patch->licenseToConfirm().empty()))
      {
        // Skipping a patch because it is marked as interactive or has
        // license to confirm and --skip-interactive is requested
        // (i.e. also --non-interactive, since it implies --skip-interactive)
        zypper.out().warning(str::form(
          // translators: %s is the name of a patch
          _("'%s' is interactive, skipping."),
          string(patch->name() + string("-") + patch->edition().asString()).c_str()));
        return false;
      }
      else
      {
        // TODO use _opts.force
        bool result = s.setToInstall();
        if (!result)
          ERR << "Marking " << s << " for installation failed" << endl;
        return result;
      }
    }
  }
  else
    XXX << "candidate patch " << patch << " is satisfied or irrelevant" << endl;

  return false;
}
