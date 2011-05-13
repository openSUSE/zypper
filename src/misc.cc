/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>
#include <sstream>
#include <boost/format.hpp>

#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"

#include "zypp/SrcPackage.h"
#include "zypp/Package.h"
#include "zypp/Capabilities.h"
#include "zypp/ui/Selectable.h"


#include "zypp/RepoInfo.h"

#include "zypp/PoolQuery.h"
#include "zypp/PoolItemBest.h"

#include "Zypper.h"
#include "main.h"
#include "utils/misc.h"
#include "utils/pager.h"
#include "utils/prompt.h"
#include "utils/getopt.h"
#include "utils/richtext.h"

#include "misc.h"

using namespace std;
using namespace zypp;
using namespace zypp::ui;
using namespace boost;

extern ZYpp::Ptr God;


void remove_selections(Zypper & zypper)
{
  // zypp gets initialized only upon the first successful processing of
  // command options, if the command was not the 'help'. bnc #372696
  if (!God)
    return;

  MIL << "Removing user selections from the solver pool" << endl;

  DBG << "Removing user setToBeInstalled()/Removed()" << endl;

  // iterate pool, searching for ResStatus::isByUser()
  // TODO optimize: remember user selections and iterate by name
  // TODO optimize: it seems this is actually needed only if the selection was
  //      not committed (user has chosen not to continue)
  const ResPool & pool = God->pool();

  for (ResPool::const_iterator it = pool.begin(); it != pool.end(); ++it)
    if (it->status().isByUser())
    {
      DBG << "Removing user setToBeInstalled()/Removed()" << endl;
      it->status().resetTransact(zypp::ResStatus::USER);
    }

  DBG << "Removing user addRequire() addConflict()" << endl;

  Resolver_Ptr solver = God->resolver();
  // FIXME port this
//   CapSet capSet = solver->getConflict();
//   for (CapSet::const_iterator it = capSet.begin(); it != capSet.end(); ++it)
//   {
//     DBG << "removing conflict: " << (*it) << endl;
//     solver->removeConflict(*it);
//   }
//   capSet = solver->getRequire();
//   for (CapSet::const_iterator it = capSet.begin(); it != capSet.end(); ++it)
//   {
//     DBG << "removing require: " << (*it) << endl;
//     solver->removeRequire(*it);
//   }

  MIL << "DONE" << endl;
}

// ----------------------------------------------------------------------------

static string get_display_name(const ResObject::constPtr & obj)
{
  // in most cases we want to display full product name (bnc #589333)
  if (obj->kind() == ResKind::product)
    return obj->summary();
  return obj->name();
}

/* debugging
static
ostream& operator << (ostream & stm, ios::iostate state)
{
  return stm << (state & ifstream::eofbit ? "Eof ": "")
	     << (state & ifstream::badbit ? "Bad ": "")
	     << (state & ifstream::failbit ? "Fail ": "")
	     << (state == 0 ? "Good ": "");
}
*/

// TODO confirm licenses
// - make this more user-friendly e.g. show only license name and
//  ask for [y/n/r] with 'r' for read the license text
//  (opened throu more or less, etc...)
// - after negative answer, call solve_and_commit() again
bool confirm_licenses(Zypper & zypper)
{
  bool confirmed = true;
  bool license_auto_agree =
    zypper.cOpts().count("auto-agree-with-licenses")
    || zypper.cOpts().count("agree-to-third-party-licenses");

  for (ResPool::const_iterator it = God->pool().begin(); it != God->pool().end(); ++it)
  {
    if (it->status().isToBeInstalled() &&
        !it->resolvable()->licenseToConfirm().empty())
    {
      ui::Selectable::Ptr selectable =
          God->pool().proxy().lookup(it->resolvable()->kind(), it->resolvable()->name());

      // this is an upgrade, check whether the license changed
      // for now we only do dumb string comparison (bnc #394396)
      if (selectable->hasInstalledObj())
      {
        bool differ = false;
        for_(inst, selectable->installedBegin(), selectable->installedEnd())
          if (inst->resolvable()->licenseToConfirm() != it->resolvable()->licenseToConfirm())
          { differ = true; break; }

        if (!differ)
        {
          DBG << "old and new license does not differ for "
              << it->resolvable()->name() << endl;
          continue;
        }
        DBG << "new license for " << it->resolvable()->name()
            << " is different, needs confirmation " << endl;
      }

      if (license_auto_agree)
      {
      	zypper.out().info(boost::str(
            // translators: the first %s is name of the resolvable,
      	    // the second is its kind (e.g. 'zypper package')
      	    format(_("Automatically agreeing with %s %s license."))
            % get_display_name(it->resolvable())
            % kind_to_string_localized(it->resolvable()->kind(),1)));

        MIL << format("Automatically agreeing with %s %s license.")
            % it->resolvable()->name() % it->resolvable()->kind().asString()
            << endl;

        continue;
      }

      ostringstream s;
      string kindstr =
        it->resolvable()->kind() != ResKind::package ?
          " (" + kind_to_string_localized(it->resolvable()->kind(), 1) + ")" :
          string();

      // introduction
      s << str::form(
          // translators: the first %s is the name of the package, the second
          // is " (package-type)" if other than "package" (patch/product/pattern)
          _("In order to install '%s'%s, you must agree"
            " to terms of the following license agreement:"),
            get_display_name(it->resolvable()).c_str(), kindstr.c_str());
      s << endl << endl;

      // license text
      const string& licenseText = it->resolvable()->licenseToConfirm();
      if (licenseText.find("DT:Rich")==licenseText.npos)
        s << licenseText;
      else
        s << processRichText(licenseText);

      // show in pager unless we are read by a machine or the pager fails
      if (zypper.globalOpts().machine_readable || !show_text_in_pager(s.str()))
        zypper.out().info(s.str(), Out::QUIET);

      // lincense prompt
      string question = _("Do you agree with the terms of the license?");
      //! \todo add 'v' option to view the license again, add prompt help
      if (!read_bool_answer(PROMPT_YN_LICENSE_AGREE, question, license_auto_agree))
      {
        confirmed = false;

        if (zypper.globalOpts().non_interactive)
        {
          zypper.out().info(
            _("Aborting installation due to the need for license confirmation."),
            Out::QUIET);
          zypper.out().info(boost::str(format(
            // translators: %sanslate the '--auto-agree-with-licenses',
            // it is a command line option
            _("Please restart the operation in interactive"
              " mode and confirm your agreement with required licenses,"
              " or use the %s option.")) % "--auto-agree-with-licenses"),
            Out::QUIET);

          MIL << "License(s) NOT confirmed (non-interactive without auto confirmation)" << endl;
        }
        else
        {
          zypper.out().info(boost::str(format(
              // translators: e.g. "... with flash package license."
              //! \todo fix this to allow proper translation
              _("Aborting installation due to user disagreement with %s %s license."))
                % get_display_name(it->resolvable())
                % kind_to_string_localized(it->resolvable()->kind(), 1)),
              Out::QUIET);
            MIL << "License(s) NOT confirmed (interactive)" << endl;
        }

        break;
      }
    }
  }

  return confirmed;
}

// ----------------------------------------------------------------------------

void report_licenses(Zypper & zypper)
{
  PoolQuery q;

  ui::Selectable::constPtr s;
  PoolItem inst;
  PoolItem inst_with_repo;

  unsigned count_installed = 0, count_installed_repo = 0, count_installed_eula = 0;
  set<string> unique_licenses;

  for_(pit, q.selectableBegin(), q.selectableEnd())
  {
    s = *pit;
    if (!s)  // FIXME this must not be necessary!
      continue;

    for_(iit, s->installedBegin(), s->installedEnd())
    {
      inst = *iit;
      ++count_installed;

      cout
        << s->name() << "-" << inst.resolvable()->edition()
        << " (" << kind_to_string_localized(s->kind(), 1) << ")"
        << endl;

      if (s->kind() == ResKind::package)
      {
        cout
          << _("License") << ": "
          << asKind<Package>(inst.resolvable())->license()
          << endl;
        unique_licenses.insert(asKind<Package>(inst.resolvable())->license());
      }

      for_(it, s->availableBegin(), s->availableEnd())
      {
        if (identical(*it, inst))
        {
          inst_with_repo = *it;
          ++count_installed_repo;
          break;
        }
      }

      if (inst_with_repo && !inst_with_repo.resolvable()->licenseToConfirm().empty())
      {
        cout << _("EULA") << ":" << endl;

        const string & licenseText =
          inst_with_repo.resolvable()->licenseToConfirm();
        if (licenseText.find("DT:Rich")==licenseText.npos)
          cout << licenseText;
        else
          cout << processRichText(licenseText);
        cout << endl;

        ++count_installed_eula;
      }
      else if (!inst.resolvable()->licenseToConfirm().empty())
        cout << "look! got an installed-only item and it has EULA! he?" << inst << endl;
      cout << "-" << endl;
    }
  }

  cout << endl << _("SUMMARY") << endl << endl;
  cout << str::form(_("Installed packages: %d"), count_installed) << endl;
  cout << str::form(_("Installed packages with counterparts in repositories: %d"), count_installed_repo) << endl;
  cout << str::form(_("Installed packages with EULAs: %d"), count_installed_eula) << endl;

  cout << str::form("Package licenses (%u):", (unsigned int) unique_licenses.size()) << endl;
  for_(it, unique_licenses.begin(), unique_licenses.end())
    cout << "* " << *it << endl;
}

// ----------------------------------------------------------------------------

static SrcPackage::constPtr source_find( const string & arg )
{
   /*
   * Workflow:
   *
   * 1. interate all SrcPackage resolvables with specified name
   * 2. find the latest version or version satisfying specification.
   */
    SrcPackage::constPtr srcpkg;

    ResPool pool(God->pool());
    DBG << "looking source for : " << arg << endl;
    for_( srcit, pool.byIdentBegin<SrcPackage>(arg),
              pool.byIdentEnd<SrcPackage>(arg) )
    {
      DBG << *srcit << endl;
      if ( ! srcit->status().isInstalled() ) // this will be true for all of the srcpackages, won't it?
      {
        SrcPackage::constPtr _srcpkg = asKind<SrcPackage>(srcit->resolvable());

        DBG << "Considering srcpakcage " << _srcpkg->name() << "-" << _srcpkg->edition() << ": ";
        if (srcpkg)
        {
          if (_srcpkg->edition() > srcpkg->edition())
          {
            DBG << "newer edition (" << srcpkg->edition() << " > " << _srcpkg->edition() << ")";
            _srcpkg.swap(srcpkg);
          }
          else
            DBG << "is older than the current candidate";
        }
        else
        {
          DBG << "first candindate";
          _srcpkg.swap(srcpkg);
        }
        DBG << endl;
      }
    }

    return srcpkg;
}

// ----------------------------------------------------------------------------

void build_deps_install(Zypper & zypper)
{
  /*
   * Workflow:
   *
   * 1. find the latest version or version satisfying specification.
   * 2. install the source package with ZYpp->installSrcPackage(SrcPackage::constPtr);
   */

  for (vector<string>::const_iterator it = zypper.arguments().begin();
       it != zypper.arguments().end(); ++it)
  {
    SrcPackage::constPtr srcpkg = source_find(*it);

    if (srcpkg)
    {
      DBG << format("Injecting build requieres for source package %s-%s")
          % srcpkg->name() % srcpkg->edition() << endl;

      // install build depenendcies only
      if (zypper.cOpts().count("build-deps-only"))
        for_(itc, srcpkg->requires().begin(), srcpkg->requires().end())
        {
          God->resolver()->addRequire(*itc);
          DBG << "requiring: " << *itc << endl;
        }
      // install the source package with build deps
      else
      {
        Capability cap(srcpkg->name(), Rel::EQ, srcpkg->edition(), ResKind::srcpackage);
        God->resolver()->addRequire(cap);
        DBG << "requiring: " << cap << endl;
      }
    }
    else
    {
      zypper.out().error(boost::str(format(
          _("Source package '%s' not found.")) % (*it)));
      zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
    }
  }
}

// ----------------------------------------------------------------------------

void mark_src_pkgs(Zypper & zypper)
{
  /*
   * Workflow:
   *
   * 1. find the latest version or version satisfying specification.
   * 2. install the source package with ZYpp->installSrcPackage(SrcPackage::constPtr);
   */

  for (vector<string>::const_iterator it = zypper.arguments().begin();
       it != zypper.arguments().end(); ++it)
  {
    SrcPackage::constPtr srcpkg = source_find(*it);

    if (srcpkg)
      zypper.runtimeData().srcpkgs_to_install.push_back(srcpkg);
    else
      zypper.out().error(boost::str(format(
          _("Source package '%s' not found.")) % (*it)));
  }
}

// ----------------------------------------------------------------------------

void install_src_pkgs(Zypper & zypper)
{
  for_(it, zypper.runtimeData().srcpkgs_to_install.begin(), zypper.runtimeData().srcpkgs_to_install.end())
  {
    SrcPackage::constPtr srcpkg = *it;
    zypper.out().info(boost::str(format(
        _("Installing source package %s-%s"))
        % srcpkg->name() % srcpkg->edition()));
    MIL << "Going to install srcpackage: " << srcpkg << endl;

    try
    {
      God->installSrcPackage(srcpkg);

      zypper.out().info(boost::str(format(
          _("Source package %s-%s successfully installed."))
          % srcpkg->name() % srcpkg->edition()));
    }
    catch (const Exception & ex)
    {
      ZYPP_CAUGHT(ex);
      zypper.out().error(ex,
        boost::str(format(_("Problem installing source package %s-%s:"))
          % srcpkg->name() % srcpkg->edition()));

      zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    }
  }
}

// ----------------------------------------------------------------------------

zypp::PoolQuery
pkg_spec_to_poolquery(const Capability & cap, const list<string> & repos)
{
  sat::Solvable::SplitIdent splid(cap.detail().name());

  PoolQuery q;
  q.addKind(splid.kind());
  q.setMatchGlob();
  for_(it, repos.begin(), repos.end())
    q.addRepo(*it);
  q.addDependency( sat::SolvAttr::name, splid.name().asString(),
		   // only package names (no provides)
		   cap.detail().op(), cap.detail().ed(),
		   // defaults to Rel::ANY (NOOP) if no versioned cap
		   Arch( cap.detail().arch() ) );
		   // defaults Arch_empty (NOOP) if no arch in cap

  DBG << "query: " << q << endl;

  return q;
}

zypp::PoolQuery
pkg_spec_to_poolquery(const Capability & cap, const string & repo)
{
  list<string> repos;
  if (!repo.empty())
    repos.push_back(repo);
  return pkg_spec_to_poolquery(cap, repos);
}

set<PoolItem>
get_installed_providers(const Capability & cap)
{
  set<PoolItem> providers;

  sat::WhatProvides q(cap);
  for_(it, q.poolItemBegin(), q.poolItemEnd())
  {
    if (traits::isPseudoInstalled( (*it).satSolvable().kind() ) )
    {
      if ( (*it).isSatisfied() )
	providers.insert( *it );
    }
    else if ( (*it).satSolvable().isSystem() )
      providers.insert( *it );
  }

  return providers;
}

string poolitem_user_string(const PoolItem & pi)
{
  return resolvable_user_string(*pi.resolvable());
}

string resolvable_user_string(const Resolvable & res)
{
  ostringstream str;
  str << res.name() << "-" << res.edition() << "." << res.arch();
  return str.str();
}

zypp::PoolItem get_installed_obj(zypp::ui::Selectable::Ptr & s)
{
  PoolItem installed;
  if (traits::isPseudoInstalled(s->kind()))
  {
    for_(it, s->availableBegin(), s->availableEnd())
      // this is OK also for patches - isSatisfied() excludes !isRelevant()
      if (it->status().isSatisfied()
          && (!installed || installed->edition() < (*it)->edition()))
        installed = *it;
  }
  else
    installed = s->installedObj();

  return installed;
}

// Local Variables:
// c-basic-offset: 2
// End:
