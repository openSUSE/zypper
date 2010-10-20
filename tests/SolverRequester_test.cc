/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

/** \file tests/SolverRequester_test.cc
 *
 * Checks whether SolverRequester issued correct zypp/solver requests given
 * specific command, package arguments and options.
 *
 * OPEN ISSUES:
 * - zypper in/up repo:package
 *   (see install12)
 *   This currently ignores vendor lock. Maybe we want this, but maybe it would
 *   be better to only allow the vendor change, if --force is added. Otherwise
 *   just inform the user. More annoying, but safer.
 * - zypper in/up packageOPversion.arch
 *   (see install13, install14)
 *   The vendor problem here as well. Maybe we want this if the request points
 *   to one specific package, e.g. 'zypper in package-version' (the same
 *   situation as above) - case install13.
 *   But if zypper has to choose from more packages, it should not change
 *   vendor, e.g. 'zypper in package>version' (case install14) should say
 *   'package-1.0.1 is available but is from different vendor. Do
 *   zypper in package-1.0.1 (or zypper in --force package>version to pick up
 *   the best version from the matching versions despite vendor change)'
 *   If we want this
 * - zypper in --best-effort name
 *   Best effort means 'require name, let solver choose the version'. This is
 *   in fact the same as 'zypper in --capability name' (uses
 *   addRequire(Capability)), but first it checks whether the 'name' matches
 *   some package name. We could deprecate this in favor of --capability...
 *
 * Proposed semantic of --force:
 * 1) if only one package matches the request, (re)install that one, despite
 *    arch change, downgrade, vendor change and similar. Just install it.
 * 2) if multiple objects match the request, (re)install the best version
 *    (the highest from the highest priority repos), despite vendor change,
 *    arch change, downgrade, etc.
 * 3) without force - don't change vendor, arch, downgrade etc, only inform user
 *    Update only if the selected version is higher than installed, and does not
 *    change vendor, and arch.
 *
 * UNAVAILABLE FUNCTIONALITY:
 * - zypper in --capability --from repo package ...
 * - zypper in --capability repo:package ...
 *   Tell solver to satisfy that requirement from specified repo(s).
 *   If we want this, we need API in libzypp/satsolver to add requirement
 *   with restriction to repo(s),
 *   e.g. Resolver::addRequire(constCapability&, set<string> repoaliases&)
 * - zypper rm [--from repo] repo:package
 *   Remove does not (nor it can) care from which repo the installed packages
 *   come from. The only thing we could implement here is, that if the installed
 *   packages have the same NEVRA conterparts in repos, only remove these. Or
 *   we would need to store the id (recently added to metadata) of the repo
 *   for the installed packages.
 *
 * INVALID REQUESTS
 * - zypper in --best-effort specific-version
 *   Best effort means 'require name, let solver choose the version'.
 *   Specifying version, arch, repo is not allowed.
 *
 * \todo test/do non-packages
 * \todo add feedback and tests for invalid requests and unavailable functions
 * \todo adapt --best-effort concept to the new package selection
 */

#include "TestSetup.h"
#include "zypp/PoolQuery.h"
#include "zypp/Locks.h"

#include "SolverRequester.h"

using namespace std;
using namespace zypp;

bool hasPoolItem(
    const set<PoolItem> & set,
    const string & name,
    const Edition & ed = Edition(),
    const Arch & arch = Arch_empty,
    const ResKind & kind = ResKind::package)
{
  for_(pit, set.begin(), set.end())
  {
    PoolItem pi(*pit);
    if (pi->name() == name &&
        (ed.empty() || ed == pi->edition()) &&
        (arch.empty() || arch == pi->arch()) &&
        kind == pi->kind())
      return true;
  }
  return false;
}

struct ApplyLock
{
  void operator()(const PoolQuery& query) const
  {
    for_(it, query.begin(), query.end())
    {
      PoolItem item(*it);
      item.status().setLock(true, ResStatus::USER);
      DBG << "lock " << item << endl;
    }
  }
};

static TestSetup test(Arch_x86_64);

BOOST_AUTO_TEST_CASE(setup)
{
  MIL << "============setup===========" << endl;
  // fake target from a subset of the online 11.1 repo
  test.loadTargetRepo(TESTS_SRC_DIR "/data/openSUSE-11.1_subset");
  test.loadRepo(TESTS_SRC_DIR "/data/openSUSE-11.1", "main");
  test.loadRepo(TESTS_SRC_DIR "/data/misc", "misc");
  test.loadRepo(TESTS_SRC_DIR "/data/OBS_zypp_svn-11.1", "zypp");

  RepoInfo repo;
  repo.setAlias("upd");
  repo.addBaseUrl(Url("file://" TESTS_SRC_DIR "/data/openSUSE-11.1_updates"));
  repo.setPriority(80);
  repo.setGpgCheck(false);
  test.loadRepo(repo);
  //test.loadRepo(TESTS_SRC_DIR "/data/openSUSE-11.1_updates", "upd");

  // resolve pool so that the satisfied status of patches/patterns becomes known
  zypp::getZYpp()->resolver()->resolvePool();
}

///////////////////////////////////////////////////////////////////////////
// install
///////////////////////////////////////////////////////////////////////////

// request : install nonsense
// response: not found by name, try caps, no cap found
BOOST_AUTO_TEST_CASE(install1)
{
  MIL << "<=============install1==============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("nonsense");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME_TRYING_CAPS));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_CAP));
}

// request : install vim
// response: vim set to install, no fallback to caps
BOOST_AUTO_TEST_CASE(install2)
{
  MIL << "<============install2===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("vim");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "vim", Edition("7.2-7.4.1"), Arch_x86_64));
}

// request : install zypper
// response: set zypper-1.0.13-0.1.1 from 11.1_updates to install (update)
BOOST_AUTO_TEST_CASE(install3)
{
  MIL << "<============install3===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("zypper");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "zypper", Edition("1.0.13-0.1.1"), Arch_x86_64));
}

// request : install netcfg
// response: already installed, no update candidate (the installed
//           is higher than any available)
BOOST_AUTO_TEST_CASE(install4)
{
  MIL << "<============install4===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("netcfg");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::ALREADY_INSTALLED));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NO_UPD_CANDIDATE));
}

// request : install info
// response: already installed (the highest available is identical to installed)
BOOST_AUTO_TEST_CASE(install5)
{
  MIL << "<============install5===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("login");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::ALREADY_INSTALLED));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NO_UPD_CANDIDATE));
}

// request : install 'perl(Net::SSL)'
// response: fall back to --capability, addRequirement 'perl(Net::SSL)'
//           (perl-Crypt-SSLeay-0.57-1.41.x86_64 from 11.1 provides it)
BOOST_AUTO_TEST_CASE(install6)
{
  MIL << "<============install6===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("perl(Net::SSL)");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME_TRYING_CAPS));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::ADDED_REQUIREMENT));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 0);
  BOOST_CHECK_EQUAL(sr.requires().size(), 1);
  BOOST_CHECK(sr.requires().find(Capability("perl(Net::SSL)")) != sr.requires().end());
}

// request : install --name 'perl(Net::SSL)'
// response: 'perl(Net::SSL)' not found
// TODO: might be nice to inform users that a provider of such capability exists
BOOST_AUTO_TEST_CASE(install7)
{
  MIL << "<============install7===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("perl(Net::SSL)");
  SolverRequester::Options sropts;
  sropts.force_by_name = true;
  SolverRequester sr(sropts);

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME_TRYING_CAPS));
}

// request : install --capability 'y2pmsh'
// response: zypper providing y2pmsh already installed (don't install package 'y2pmsh')
BOOST_AUTO_TEST_CASE(install8)
{
  MIL << "<============install8===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("y2pmsh");
  SolverRequester::Options sropts;
  sropts.force_by_cap = true;
  SolverRequester sr(sropts);

  sr.install(rawargs);

  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME_TRYING_CAPS));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::ALREADY_INSTALLED));
  BOOST_CHECK(sr.toInstall().empty());
  BOOST_CHECK(sr.requires().empty());
}

// request : install 'info'
// response: Update to info-4.12-1.111. Update candidate
//           info-4.13-1.1 is available, too, but has different vendor.
BOOST_AUTO_TEST_CASE(install9)
{
  MIL << "<============install9===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("info");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "info", Edition("4.12-1.111"), Arch_x86_64));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::UPD_CANDIDATE_CHANGES_VENDOR));
}

// request : install stallarium
// response: already installed, no update candidate (no available objects
//           in repos)
BOOST_AUTO_TEST_CASE(install10)
{
  MIL << "<============install10===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("stellarium");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::ALREADY_INSTALLED));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_IN_REPOS));
}

// request : install diffutils
// response: Already installed. Update candidate diffutils-2.9.0-1 is available
//           but has different vendor.
BOOST_AUTO_TEST_CASE(install11)
{
  MIL << "<============install11===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("diffutils");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::ALREADY_INSTALLED));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK(sr.toInstall().empty());
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::UPD_CANDIDATE_CHANGES_VENDOR));
}


// request : install info-4.13-1.1
// response: Already installed. Update to info-4.13-1.1 despite the vendor
//           change.
BOOST_AUTO_TEST_CASE(install12)
{
  MIL << "<============install12===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("info-4.13-1.1");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "info", Edition("4.13-1.1"), Arch_x86_64));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::UPD_CANDIDATE_CHANGES_VENDOR));
}

// request : install misc:info
// response: Already installed. Update to info-4.13-1.1 from misc, despite the
//           vendor change.
BOOST_AUTO_TEST_CASE(install13)
{
  MIL << "<============install13===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("misc:info");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "info", Edition("4.13-1.1"), Arch_x86_64));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::UPD_CANDIDATE_CHANGES_VENDOR));
}

// request : install info>4.12-1.109.
// response: Already installed. Update to info-4.13-1.1 from misc, despite the
//           vendor change.
BOOST_AUTO_TEST_CASE(install14)
{
  MIL << "<============install14===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("info>4.12-1.109");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "info", Edition("4.13-1.1"), Arch_x86_64));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::UPD_CANDIDATE_CHANGES_VENDOR));
}

// bnc #600471
// request : install openSUSE-release-usb-x11
// response: 'openSUSE-release-usb-x11' not found
//           should not fall back to caps, because
BOOST_AUTO_TEST_CASE(install15)
{
  MIL << "<============install15===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("openSUSE-release-usb-x11");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME_TRYING_CAPS));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_CAP));
  BOOST_CHECK(sr.toInstall().empty());
}


///////////////////////////////////////////////////////////////////////////
// Locks

// request : install zypper
// response: Already installed & locked. Update candidate (zypper-1.0.13-0.1.1)
//           exists but can't be installed.
/*
BOOST_AUTO_TEST_CASE(install100)
{
  MIL << "<============install100===============>" << endl;

  PoolQuery q;
  q.addAttribute(sat::SolvAttr::name, "zypper");
  q.setMatchExact();

  ApplyLock()(q);

  vector<string> rawargs;
  rawargs.push_back("zypper");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::ALREADY_INSTALLED));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK(sr.toInstall().empty());
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::UPD_CANDIDATE_IS_LOCKED));

  // RemoveLock()(q);
}
*/
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Repo Priority

// request : install cron
// response: install cron-4.1-194.33.1, report newer available (4.1-195.0), but
//           from lower-priority repo.
BOOST_AUTO_TEST_CASE(install200)
{
  MIL << "<============install200===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("cron");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "cron", Edition("4.1-194.33.1"), Arch_x86_64));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::UPD_CANDIDATE_HAS_LOWER_PRIO));
}

// request : install cron-4.1-195.0.x86_64
// response: install (update to) cron-4.1-195.0.x86_64, despite that there's
//           another version in a higher-priority repo (upd).
// see OPEN ISSUES: should this work only with --force?
BOOST_AUTO_TEST_CASE(install201)
{
  MIL << "<============install201===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("cron-4.1-195.0.x86_64");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "cron", Edition("4.1-195.0"), Arch_x86_64));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::UPD_CANDIDATE_HAS_LOWER_PRIO));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::UPD_CANDIDATE_CHANGES_VENDOR));
}
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// Patches

// request : install patch:dbus-1
// response: install needed patch:dbus-1-717
BOOST_AUTO_TEST_CASE(install300)
{
  MIL << "<============install300===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("patch:dbus-1");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "dbus-1", Edition("717"), Arch(), ResKind::patch));
}

// request : install patch:imap
// response: the patch is not needed
BOOST_AUTO_TEST_CASE(install301)
{
  MIL << "<============install301===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("patch:imap");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK(sr.toInstall().empty());
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::PATCH_NOT_NEEDED));
}

// request : install patch:libxml2-434
// response: the patch is already satisfied, but newer exists
BOOST_AUTO_TEST_CASE(install302)
{
  MIL << "<============install302===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("patch:libxml2-434");
  SolverRequester sr;
  //zypp::getZYpp()->resolver()->createSolverTestcase(TESTS_BUILD_DIR "/testcase");

  sr.install(rawargs);

  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK(sr.toInstall().empty());
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::ALREADY_INSTALLED));
}

// request : install patch:libxml2
// response: install needed patch:libxml2-1175
BOOST_AUTO_TEST_CASE(install303)
{
  MIL << "<============install303===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("patch:libxml2");
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "libxml2", Edition("1175"), Arch(), ResKind::patch));
}

// TODO install locked patch
// TODO install all needed patches (- locked)
// TODO install all needed patches of specified category

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// Force (the --force)

// request : install --force cron
// response: force install of cron-4.1-195.0, despite lower-priority repo.
BOOST_AUTO_TEST_CASE(install400)
{
  MIL << "<============install400===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("cron");
  SolverRequester::Options sropts;
  sropts.force = true;
  SolverRequester sr(sropts);

  sr.install(rawargs);

  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::ALREADY_INSTALLED));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::FORCED_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "cron", Edition("4.1-195.0"), Arch_x86_64));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::UPD_CANDIDATE_HAS_LOWER_PRIO));
}

// request : install --force info
// response: force install of info-4.13-1.1, despite changed vendor.
BOOST_AUTO_TEST_CASE(install401)
{
  MIL << "<============install401===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("info");
  SolverRequester::Options sropts;
  sropts.force = true;
  SolverRequester sr(sropts);

  sr.install(rawargs);

  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::ALREADY_INSTALLED));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::FORCED_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "info", Edition("4.13-1.1"), Arch_x86_64));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::UPD_CANDIDATE_CHANGES_VENDOR));
}

// request : install --force netcfg
// response: already installed, no update candidate, reinstall with
//           highest available (netcfg-11.0.42-22.5 => downgrade)
BOOST_AUTO_TEST_CASE(install402)
{
  MIL << "<============install402===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("netcfg");
  SolverRequester::Options sropts;
  sropts.force = true;
  SolverRequester sr(sropts);

  sr.install(rawargs);

  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::ALREADY_INSTALLED));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::FORCED_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "netcfg", Edition("11.0.42-22.5"), Arch_noarch));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::NO_UPD_CANDIDATE));
}
// request : install --force sed
// response: already installed, no update candidate, reinstall
BOOST_AUTO_TEST_CASE(install403)
{
  MIL << "<============install403===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("popt");
  SolverRequester::Options sropts;
  sropts.force = true;
  SolverRequester sr(sropts);

  sr.install(rawargs);

  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::ALREADY_INSTALLED));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::FORCED_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "popt", Edition("1.7-5.5"), Arch_x86_64));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::NO_UPD_CANDIDATE));
}
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// remove
///////////////////////////////////////////////////////////////////////////

// request : remove nonsense
// response: not found by name, try caps, no cap found
BOOST_AUTO_TEST_CASE(remove1)
{
  MIL << "<===========remove1================>" << endl;
  PackageArgs::Options argopts;
  argopts.do_by_default = false;

  vector<string> rawargs;
  rawargs.push_back("nonsense");
  PackageArgs args(rawargs, ResKind::package, argopts);
  SolverRequester sr;

  sr.remove(args);

  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME_TRYING_CAPS));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_CAP));
}

// request : remove --name nonsense
// response: not found by name. Don't try caps.
BOOST_AUTO_TEST_CASE(remove2)
{
  MIL << "<============remove2===============>" << endl;
  PackageArgs::Options argopts;
  argopts.do_by_default = false;

  vector<string> rawargs;
  rawargs.push_back("nonsense");
  PackageArgs args(rawargs, ResKind::package, argopts);

  SolverRequester::Options sropts;
  sropts.force_by_name = true;
  SolverRequester sr1(sropts);

  sr1.remove(args);
  BOOST_CHECK(sr1.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME));
  BOOST_CHECK(!sr1.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME_TRYING_CAPS));
  BOOST_CHECK(!sr1.hasFeedback(SolverRequester::Feedback::NOT_FOUND_CAP));
}

// request : remove mc
// response: not installed, fall back to caps, no provider installed
BOOST_AUTO_TEST_CASE(remove3)
{
  MIL << "<============remove3===============>" << endl;
  PackageArgs::Options argopts;
  argopts.do_by_default = false;

  vector<string> rawargs;
  rawargs.push_back("mc");
  PackageArgs args(rawargs, ResKind::package, argopts);
  SolverRequester sr;

  sr.remove(args);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_INSTALLED));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME_TRYING_CAPS));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NO_INSTALLED_PROVIDER));
}

// request : remove mc
// response: not installed, fall back to caps, no provider installed
// this one is done by sr.remove(vector<string>) instead of PackageArgs
BOOST_AUTO_TEST_CASE(remove4)
{
  MIL << "<==============remove4=============>" << endl;
  // beware of implicit conversion from vector<string> to PackageArgs
  // if not avoided, the resulting PackageArgs would have
  // PackageArgs::Options::do_by_default == true! => args without +/- modifiers
  // would default to install/doCaps, not remove/dontCaps!
  vector<string> rawargs;
  rawargs.push_back("mc");
  SolverRequester sr;

  sr.remove(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_INSTALLED));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME_TRYING_CAPS));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NO_INSTALLED_PROVIDER));
}

// request : remove --name mc
// response: not installed. Don't fall back to caps.
BOOST_AUTO_TEST_CASE(remove5)
{
  MIL << "<=============remove5==============>" << endl;
  vector<string> rawargs;
  rawargs.push_back("mc");
  SolverRequester::Options sropts;
  sropts.force_by_name = true;
  SolverRequester sr(sropts);

  sr.remove(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_INSTALLED));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME_TRYING_CAPS));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::NO_INSTALLED_PROVIDER));
}

// request : remove libzypp
// response: libzypp marked for removal
BOOST_AUTO_TEST_CASE(remove6)
{
  MIL << "<=============remove6==============>" << endl;
  vector<string> rawargs;
  rawargs.push_back("libzypp");
  SolverRequester sr;

  sr.remove(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::SET_TO_REMOVE));
  BOOST_CHECK_EQUAL(sr.toRemove().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toRemove(), "libzypp", Edition("5.24.5-1.1"), Arch_x86_64));
  BOOST_CHECK(sr.conflicts().empty());
}

// request : remove --capability y2pmsh
// response: conflict 'y2pmsh' added despite a package named y2pmsh exists;
//           the installed 'zypper' provides 'y2pmsh'
BOOST_AUTO_TEST_CASE(remove7)
{
  MIL << "<=============remove7==============>" << endl;
  vector<string> rawargs;
  rawargs.push_back("y2pmsh");

  SolverRequester::Options sropts;
  sropts.force_by_cap = true;
  SolverRequester sr(sropts);

  sr.remove(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::ADDED_CONFLICT));
  BOOST_CHECK(sr.toRemove().empty());
  BOOST_CHECK_EQUAL(sr.conflicts().size(), 1);
  BOOST_CHECK(sr.conflicts().find(Capability("y2pmsh")) != sr.conflicts().end());
}

// request : remove onekit
// response: set onekit for removal.
//           must not add conflict 'onekit' ('onekit' exists, must mark by name
//           first. Conflict 'onekit' would also remove 'newkit' which provides
//           'onekit')
// bnc #458318
BOOST_AUTO_TEST_CASE(remove8)
{
  MIL << "<=============remove8==============>" << endl;
  vector<string> rawargs;
  rawargs.push_back("onekit");
  SolverRequester sr;

  sr.remove(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::SET_TO_REMOVE));
  BOOST_CHECK_EQUAL(sr.toRemove().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toRemove(), "onekit", Edition("0.0.2-1"), Arch_x86_64));
  BOOST_CHECK(sr.conflicts().empty());
}


///////////////////////////////////////////////////////////////////////////
// update
///////////////////////////////////////////////////////////////////////////
//
// Update is basically the same as install, except that in case the
// requested package is not installed, update says NOT_INSTALLED or
// NO_INSTALLED_PROVIDER whereas install says SET_TO_INSTALL. In case the
// requested package is installed, update just performs the update; install
// says ALREADY_INSTALLED (in case no update is selected) or performs
// the update.
//
// In other words:
// install == get the best available version to system (while abiding policies)
//            This means also update.
// update  == the same as install, but only if some version of the package is
//            already installed. If not, just say so and quit.
///////////////////////////////////////////////////////////////////////////


// request : update vim
// response: not installed
BOOST_AUTO_TEST_CASE(update1)
{
  MIL << "<============update1===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("vim");
  PackageArgs args(rawargs);
  SolverRequester sr;

  sr.update(rawargs);
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_INSTALLED));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME_TRYING_CAPS));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::NO_INSTALLED_PROVIDER));
}

// request : update nonsense
// response: not found, fall back to caps, no provider found
BOOST_AUTO_TEST_CASE(update2)
{
  MIL << "<============update2===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("nonsense");
  PackageArgs args(rawargs);
  SolverRequester sr;

  sr.update(rawargs);
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME_TRYING_CAPS));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_CAP));
}

// response: vim set to install, no fallback to caps
BOOST_AUTO_TEST_CASE(update3)
{
  MIL << "<============update3===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("info");
  SolverRequester sr;

  sr.update(rawargs);

  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::ALREADY_INSTALLED));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "info", Edition("4.12-1.111"), Arch_x86_64));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::UPD_CANDIDATE_CHANGES_VENDOR));
}


// request :
// response:
/*BOOST_AUTO_TEST_CASE(installX)
{
  MIL << "<===========================>" << endl;

  vector<string> rawargs;
  rawargs.push_back("");
  SolverRequester::Options sropts;
  sropts.force_by_name = true;
  SolverRequester sr(sropts);

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_INSTALLED));
}
*/
