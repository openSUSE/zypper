#include <boost/format.hpp>
//#include <iostream>
#include "zypp/ZYppFactory.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/Algorithm.h"
#include "zypp/base/Functional.h"
#include "zypp/Filter.h"
#include "zypp/PoolQuery.h"
#include "zypp/PoolItemBest.h"

#include "utils/misc.h"

#include "install.h"
#include "update.h"
#include "repos.h"
#include "PackageArgs.h"

using namespace std;
using namespace zypp;
using namespace boost;

extern ZYpp::Ptr God;


static PoolItem findInstalledItemInRepos(const PoolItem & installed)
{
  const zypp::ResPool & pool(zypp::ResPool::instance());
  PoolItem result;
  invokeOnEach(
    pool.byIdentBegin(installed), pool.byIdentEnd(installed),
    functor::chain(
      filter::SameItemAs(installed),
      resfilter::ByUninstalled()),
    functor::getFirst(result));
  XXX << "findInstalledItemInRepos(" << installed << ") => " << result << endl;
  return result;
}

// TODO edition - need solver API
static void
mark_for_install(Zypper & zypper,
                      const ResObject::Kind &kind,
                      const string &name,
                      const string & repo = "",
                      const string & arch = "",
                      bool report_already_installed = true)
{
  // name and kind match:
  DBG << "Iterating over [" << kind << "] " << name;
  if (!repo.empty())
    DBG << ", repo: " << repo;
  if (!arch.empty())
    DBG << ", arch: " << arch;
  DBG << "...";

  PoolQuery q;
  q.addAttribute(sat::SolvAttr::name, name);
  q.addKind(kind);
  if (!repo.empty())
    q.addRepo(repo);
  if (!arch.empty())
    q.addAttribute(sat::SolvAttr::arch, arch);
  q.setCaseSensitive(false);
  q.setMatchExact();

//  if (q.empty())
  if (q.begin() == q.end())
  {
    DBG << "... done" << endl;
    zypper.out().error(
      // translators: meaning a package %s or provider of capability %s
      str::form(_("'%s' not found"), name.c_str()));
    WAR << str::form("'%s' not found", name.c_str()) << endl;
    zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
    if (zypper.globalOpts().non_interactive)
      ZYPP_THROW(ExitRequestException());
    return;
  }

  ui::Selectable::Ptr s = *q.selectableBegin();
  DBG << "... done" << endl;

  bool force = copts.count("force");


  PoolItem candidate = s->updateCandidateObj();
  if (!candidate)
    candidate = s->installedObj();

  if (s->installedObj() &&
      identical(s->installedObj(), candidate) &&
      !force)
  {
    // if it is broken install anyway, even if it is installed
    if (candidate.isBroken())
      candidate.status().setToBeInstalled(zypp::ResStatus::USER);
    else if (report_already_installed)
      zypper.out().info(boost::str(format(
        // translators: e.g. skipping package 'zypper' (the newest version already installed)
        //! \todo capitalize the first letter
        _("skipping %s '%s' (the newest version already installed)"))
        % kind_to_string_localized(kind,1) % name));
  }
  else
  {
    if (force && !s->installedEmpty())
    {
      PoolItem repoitem = findInstalledItemInRepos(s->installedObj());
      if (!repoitem)
      {
        zypper.out().info(str::form(
            // translators: %s-%s.%s is name-version.arch
            _("Package %s-%s.%s not found in repositories, cannot reinstall."),
            s->name().c_str(), s->installedObj().resolvable()->edition().c_str(),
            s->installedObj().resolvable()->arch().c_str()));
        zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
        return;
      }
      else if (candidate.status().isInstalled())
        candidate = repoitem;
    }

    if (!candidate.status().setToBeInstalled(zypp::ResStatus::USER) && !force)
    {
        zypper.out().error(boost::str(
          format(_("Failed to add '%s' to the list of packages to be installed."))
          % name));
        ERR << "Could not set " << name << " as to-be-installed" << endl;
    }
  }
}

// ----------------------------------------------------------------------------

struct DeleteProcess
{
  bool found;
  DeleteProcess ()
    : found(false)
  { }

  bool operator() ( const PoolItem& provider )
  {
    found = true;
    DBG << "Marking for deletion: " << provider << endl;
    bool result = provider.status().setToBeUninstalled( zypp::ResStatus::USER );
    if (!result) {
      Zypper::instance()->out().error(boost::str(format(
          _("Failed to add '%s' to the list of packages to be removed."))
          % provider.resolvable()->name()));
      ERR << "Could not set " << provider.resolvable()->name()
          << " as to-be-uninstalled" << endl;
    }
    return true;                // get all of them
  }
};

// ----------------------------------------------------------------------------

// mark all matches
static void mark_for_uninstall(Zypper & zypper,
                        const ResObject::Kind &kind,
                        const std::string &name)
{
  const ResPool &pool = God->pool();
  // name and kind match:

  DeleteProcess deleter;
  DBG << "Iterating over " << name << endl;
  invokeOnEach( pool.byIdentBegin( kind, name ),
                pool.byIdentEnd( kind, name ),
                resfilter::ByInstalled(),
                zypp::functor::functorRef<bool,const zypp::PoolItem&> (deleter)
                );
  DBG << "... done" << endl;
  if (!deleter.found) {
    zypper.out().error(
      // translators: meaning a package %s or provider of capability %s
      str::form(_("'%s' not found"), name.c_str()));
    WAR << str::form("'%s' not found", name.c_str()) << endl;
    zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
    return;
  }
}

// ----------------------------------------------------------------------------

static void
mark_by_name (Zypper & zypper,
              bool install_not_remove,
              const ResObject::Kind &kind,
              const string &name,
              const string & repo = "",
              const string & arch = "",
              bool report_already_installed = true) // might be a CLI option
{
  if (install_not_remove)
    mark_for_install(zypper, kind, name, repo, arch, report_already_installed);
  else
    mark_for_uninstall(zypper, kind, name);
}


// don't try NAME-EDITION yet, could be confused by
// dbus-1-x11, java-1_4_2-gcj-compat, ...
/*

ma@: Look at the Capability::guessPackageSpec implementaion. This might be
what you want unless you also want to support globbing like 'libz*-12.3-14'.
In this case you need PoolQuery instead of WhatProvides lookups.

There is no rule that an edition starts with a number, so your regex approach
won't work. If your string is correctly parsed as a name-capability, you can
check whether it matches a package name. If not, replace the last '-' by a '=',
and check whether the namepart now matches a package (-versionwithoutrelease).
If not, replace the one but last '-' and try again (-version-release).

That's basically what guessPackageSpec does, but it also supports embeded
arch specs: "libzypp-1.2.3-4.5.arch" or "libzypp.arch-1.2.3-4.5".

bool mark_by_name_edition (...)
  static const regex rx_name_edition("(.*?)-([0-9].*)");

  smatch m;
  if (! is_cap && regex_match (capstr, m, rx_name_edition)) {
    capstr = m.str(1) + " = " + m.str(2);
    is_cap = true;
  }
*/

// ----------------------------------------------------------------------------

static void
mark_by_capability (Zypper & zypper,
                    bool install_not_remove,
                    const ResKind & kind,
                    const Capability & cap)
{
  if (!cap.empty()) {
    DBG << "Capability: " << cap << endl;

    Resolver_Ptr resolver = zypp::getZYpp()->resolver();
    if (install_not_remove) {
      DBG << "Adding requirement " << cap << endl;
      resolver->addRequire (cap);
    }
    else {
      DBG << "Adding conflict " << cap << endl;
      resolver->addConflict (cap);
    }
  }
}

// ----------------------------------------------------------------------------

// join arguments at comparison operators ('=', '>=', and the like)
static void
install_remove_preprocess_args(const Zypper::ArgList & args,
                               Zypper::ArgList & argsnew)
{
  Zypper::ArgList::size_type argc = args.size();
  argsnew.reserve(argc);
  string tmp;
  // preprocess the arguments
  for(Zypper::ArgList::size_type i = 0, lastnew = 0; i < argc; ++i)
  {
    tmp = args[i];
    if (i
        && (tmp == "=" || tmp == "==" || tmp == "<"
            || tmp == ">" || tmp == "<=" || tmp == ">=")
        && i < argc - 1)
    {
      argsnew[lastnew-1] += tmp + args[++i];
      continue;
    }
    else if (tmp.find_last_of("=<>") == tmp.size() - 1 && i < argc - 1)
    {
      argsnew.push_back(tmp + args[++i]);
      ++lastnew;
    }
    else if (i && tmp.find_first_of("=<>") == 0)
    {
      argsnew[lastnew-1] += tmp;
      ++i;
    }
    else
    {
      argsnew.push_back(tmp);
      ++lastnew;
    }
  }

  DBG << "old: ";
  copy(args.begin(), args.end(), ostream_iterator<string>(DBG, " "));
  DBG << endl << "new: ";
  copy(argsnew.begin(), argsnew.end(), ostream_iterator<string>(DBG, " "));
  DBG << endl;
}

// ----------------------------------------------------------------------------

static void
mark_selectable(Zypper & zypper,
                ui::Selectable & s,
                bool install_not_remove,
                bool force,
                const string & repo = "",
                const string & arch = "")
{
  PoolItem theone = s.updateCandidateObj();
  if (!theone)
    theone = s.installedObj();

  DBG << "the One: " << theone << endl;

  //! \todo handle multiple installed case
  bool theoneinstalled; // is the One installed ?
  if (!traits::isPseudoInstalled(s.kind()))
    theoneinstalled = !s.installedEmpty() &&
      identical(s.installedObj(), theone);
  else if (s.kind() == ResKind::patch)
    theoneinstalled = theone.isRelevant() && theone.isSatisfied();
  else
    theoneinstalled = theone.isSatisfied();

  bool anyinstalled = theoneinstalled;
  PoolItem installed;
  if (theoneinstalled)
    installed = theone;
  else
  {
    if (!traits::isPseudoInstalled(s.kind()))
    {
      anyinstalled = s.hasInstalledObj();
      installed = s.installedObj();
    }
    else if (s.kind() == ResKind::patch)
    {
      for_(it, s.availableBegin(), s.availableEnd())
      {
        if (it->isRelevant() && it->isSatisfied())
        {
          if (installed)
          {
            if (it->resolvable()->edition() > installed->edition())
              installed = *it;
          }
          else
          {
            installed = *it;
            anyinstalled = true;
          }
        }
      }
    }
    else
    {
      for_(it, s.availableBegin(), s.availableEnd())
      {
        if (it->status().isSatisfied())
        {
          if (installed)
          {
            if (it->resolvable()->edition() > installed->edition())
              installed = *it;
          }
          else
          {
            installed = *it;
            anyinstalled = true;
          }
        }
      }
    }
  }

  if (install_not_remove)
  {
    if (theoneinstalled && !force)
    {
      DBG << "the One (" << theone << ") is already installed, skipping." << endl;
      zypper.out().info(str::form(
          _("'%s' is already installed."), s.name().c_str()));

      // report why an update was not found for the package (bnc #522223)
      if (!traits::isPseudoInstalled(s.kind()))
        selectable_update_report(zypper, s);

      return;
    }

    if (theoneinstalled && force)
    {
      s.setStatus(ui::S_Install);
      DBG << s << " install: forcing reinstall" << endl;
    }
    else
    {
      Capability c;

      // require version greater than the installed. The solver should pick up
      // the latest installable in this case, which is what we want for all
      // kinds (and for patches having the latest installed should automatically
      // satisfy all older version, or make them irrelevant)

      //! could be a problem for patches if there would a greater version
      //! of a patch appear that would be irrelevant at the same time. Should
      //! not happen usually.

      //! this causes bnc #539360 if wrong update candidate is found
      //! this request installation of installed > installed_version, which
      //! can drag in a package with higher version even from repo with lower
      //! priority
      //! hopefully using ui::Selectable::updateCandidateObj() will fix this
      if (anyinstalled)
        c = Capability(s.name(), Rel::GT, installed->edition(), s.kind());
      // require any version
      else
        c = Capability(s.name(), s.kind());

      God->resolver()->addRequire(c);
      DBG << s << " install: adding requirement " << c << endl;
    }
  }
  // removing is simpler, as usually
  //! \todo but not that simple - simply adding a conflict with a pattern
  //! or patch does not make the packages it requires to be removed.
  //! we still need to define what response of zypper -t foo bar should be for PPP
  else
  {
    if (!anyinstalled)
    {
      zypper.out().info(str::form(
          _("'%s' is not installed."), s.name().c_str()));
      DBG << s << " remove: not installed, skipping." << endl;
      return;
    }

    Capability c(s.name(), s.kind());
    God->resolver()->addConflict(c);
    DBG << s << " remove: adding conflict " << c << endl;
  }
}

// ----------------------------------------------------------------------------

void install_remove(Zypper & zypper,
                    const Zypper::ArgList & args,
                    bool install_not_remove,
                    const ResKind & kind)
{
  if (args.empty())
    return;

  if (kind == ResKind::patch && !install_not_remove)
  {
    zypper.out().error(
        _("Cannot uninstall patches."),
        _("Installed status of a patch is determined solely based on its dependencies.\n"
          "Patches are not installed in sense of copied files, database records,\n"
          "or similar."));
    zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    throw ExitRequestException("not implemented");
  }

  if (kind == ResKind::pattern && !install_not_remove)
  {
    //! \todo define and implement pattern removal (bnc #407040)
    zypper.out().error(
        _("Uninstallation of a pattern is currently not defined and implemented."));
    zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    throw ExitRequestException("not implemented");
  }

  bool force_by_capability = zypper.cOpts().count("capability");
  bool force_by_name = zypper.cOpts().count("name");
  bool force = zypper.cOpts().count("force");
  if (force)
    force_by_name = true;

  if (force_by_capability && force_by_name)
  {
    zypper.out().error(boost::str(
      format(_("%s contradicts %s")) % "--capability" % (force? "--force" : "--name")));

    zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    ZYPP_THROW(ExitRequestException());
  }

  if (install_not_remove && force_by_capability && force)
  {
    // translators: meaning --force with --capability
    zypper.out().error(boost::str(format(_("%s cannot currently be used with %s"))
      % "--force" % "--capability"));
    zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    ZYPP_THROW(ExitRequestException());
  }

  Zypper::ArgList argsnew;
  install_remove_preprocess_args(args, argsnew);

  // --from

  parsed_opts::const_iterator optit;
  if (install_not_remove &&
      (optit = zypper.cOpts().find("from")) != zypper.cOpts().end())
  {
    list<string> not_found;
    list<RepoInfo> repos;
    get_repos(zypper, optit->second.begin(), optit->second.end(), repos, not_found);
    if (!not_found.empty())
    {
      report_unknown_repos(zypper.out(), not_found);
      zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      return;
    }

    // for each argument search (glob) & mark
    for_(strit, argsnew.begin(), argsnew.end())
    {
      PoolQuery q;
      q.addKind(kind);
      q.addAttribute(sat::SolvAttr::name, *strit);
      for_( it, repos.begin(), repos.end() )
      {
        q.addRepo(it->alias());
      }
      q.setMatchGlob();

      // Get the best matching items and tag them for
      // installation.
      PoolItemBest bestMatches( q.begin(), q.end() );
      if ( ! bestMatches.empty() )
      {
        for_( sit, bestMatches.begin(), bestMatches.end() )
        {
          ui::asSelectable()( *sit )->setOnSystem( *sit, ResStatus::USER );
        }
      }
      else
      {
        // translators: meaning a package %s or provider of capability %s
        zypper.out().error(str::form(_("'%s' not found."), strit->c_str()));
        WAR << str::form("'%s' not found", strit->c_str()) << endl;
        zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
        if (zypper.globalOpts().non_interactive)
          ZYPP_THROW(ExitRequestException());
      }
    }
    return;
  }

  PackageArgs pargs(args);
  for_(it, pargs.doCaps().begin(), pargs.doCaps().end())
  {
    // For given PackageArgs:
    //
    // 1) if the capability only contains name and kind, and --capability was
    //    not specified, or --name was specified, try to install 'by-name'
    //    first, i.e. via ui::Selectable and/or PoolItem.
    //    note: wildcards must be supported here, use PoolQuery with match_glob
    //          to find the selectables
    //
    // 2) if no package could be found by name, or the capability contains
    //    op+version/arch, or --capability was specified,
    //    install 'by-capability', i.e. using ResPool::addRequires(cap)
    //
    // NOTES
    // * In both cases check for already installed packages, available
    //   candidates, and report any problems back to user.
    // * If the argument contains repository, issue request forcing the repo and
    //   - if --force was specified, insist on installing from that repo (even
    //     downgrade or change vendor   TODO
    //   - if --force was NOT used, only install new, or upgrade without vendor
    //     change. If downgrade or vendor change is needed, report back to user
    //     and advice to use --force, if that's what is really wanted. TODO

    sat::Solvable::SplitIdent splid(it->first.detail().name());
    ResKind capkind = splid.kind();
    string capname = splid.name().asString();

    // mark by name by force
    if (force_by_name)
    {
      //! \todo FIXME this does not work: mark_by_name does not pick the arch nor repo. Make an API in zypp for this
      mark_by_name (zypper, true, capkind, capname, it->second, it->first.detail().arch().asString());
      continue;
    }

    // try to install by name first (if version or arch was not specified)
    if (!(force_by_capability || it->first.detail().hasArch() || it->first.detail().isVersioned()))
    {
      PoolQuery q;
      q.addKind(kind);
      q.addAttribute(sat::SolvAttr::name, capname);
      q.setMatchGlob();
      bool found = false;
      for_(s, q.selectableBegin(), q.selectableEnd())
      {
        //! FIXME mark_selectable works via addRequires, change it to mark via selectable!!!
        // otherwise the solver is free to install any provider of the symbol
        mark_selectable(zypper, **s, true, force);
        found = true;
      }
      // done with this requirement, skip to next argument
      if (found)
        continue;
    }

    // try by capability
    sat::WhatProvides q(it->first);

    // is there a provider for the requested capability?
    if (q.empty())
    {
      // translators: meaning a package %s or provider of capability %s
      zypper.out().error(str::form(_("'%s' not found."), it->first.asString().c_str()));
      WAR << str::form("'%s' not found", it->first.asString().c_str()) << endl;
      zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
      if (zypper.globalOpts().non_interactive)
        ZYPP_THROW(ExitRequestException());
      continue;
    }

    // is the provider already installed?
    bool installed = false;
    string provider;
    for_(solvit, q.poolItemBegin(), q.poolItemEnd())
    {
      if (traits::isPseudoInstalled(solvit->resolvable()->kind()))
        installed = solvit->isSatisfied();
      else
        installed = solvit->status().isInstalled();
      if (installed)
      {
        provider = solvit->resolvable()->name();
        break;
      }
    }

    // already installed, nothing to do
    if (installed)
    {
      if (provider == capname)
        zypper.out().info(str::form(
            _("'%s' is already installed."), capname.c_str()));
      else
        zypper.out().info(str::form(
            // translators: %s are package names
            _("'%s' providing '%s' is already installed."),
            provider.c_str(), it->first.asString().c_str()));

      MIL << str::form("skipping '%s': already installed", it->first.asString().c_str()) << endl;
      continue;
    }

    mark_by_capability (zypper, true, capkind, it->first);
  }

  for_(it, pargs.dontCaps().begin(), pargs.dontCaps().end())
  {
    sat::Solvable::SplitIdent splid(it->first.detail().name());
    ResKind capkind = splid.kind();
    string capname = splid.name().asString();

    // mark by name by force
    if (force_by_name)
    {
      mark_by_name (zypper, true, capkind, capname,
          it->second, it->first.detail().arch().asString());
      continue;
    }

    // try to remove by name first (if version or arch was not specified)
    if (!(force_by_capability || it->first.detail().hasArch() || it->first.detail().isVersioned()))
    {
      PoolQuery q;
      q.addKind(kind);
      q.addAttribute(sat::SolvAttr::name, capname);
      q.setMatchGlob();
      bool found = false;
      for_(s, q.selectableBegin(), q.selectableEnd())
      {
        mark_selectable(zypper, **s, false, force);
        found = true;
      }
      // done with this requirement, skip to next argument
      if (found)
        continue;
    }

    // try by capability
    sat::WhatProvides q(it->first);

    // is there a provider for the requested capability?
    if (q.empty())
    {
      // translators: meaning a package %s or provider of capability %s
      zypper.out().error(str::form(_("'%s' not found."), it->first.asString().c_str()));
      WAR << str::form("'%s' not found", it->first.asString().c_str()) << endl;
      zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
      if (zypper.globalOpts().non_interactive)
        ZYPP_THROW(ExitRequestException());
      continue;
    }

    // is the provider installed?
    bool installed = false;
    string provider;
    for_(solvit, q.poolItemBegin(), q.poolItemEnd())
    {
      if (traits::isPseudoInstalled(solvit->resolvable()->kind()))
        installed = solvit->isSatisfied();
      else
        installed = solvit->status().isInstalled();
      if (installed)
      {
        provider = solvit->resolvable()->name();
        break;
      }
    }

    // not installed, nothing to do
    if (!installed)
    {
      // translators: meaning a package %s or provider of capability %s
      zypper.out().info(str::form(_("'%s' is not installed."), capname.c_str()));
      MIL << str::form("skipping '%s': not installed", capname.c_str()) << endl;
      continue;
    }

    mark_by_capability (zypper, false, capkind, it->first);
  }
}
