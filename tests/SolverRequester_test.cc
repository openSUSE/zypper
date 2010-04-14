/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "TestSetup.h"

#include "SolverRequester.h"

using namespace std;
using namespace zypp;

bool hasPoolItem(
    const set<PoolItem> & set,
    const string & name,
    const Edition & ed = Edition(),
    const Arch & arch = Arch_empty)
{
  for_(pit, set.begin(), set.end())
  {
    PoolItem pi(*pit);
    if (pi->name() == name &&
        (ed.empty() || ed == pi->edition()) &&
        (arch.empty() || arch == pi->arch()))
      return true;
  }
  return false;
}

BOOST_AUTO_TEST_CASE(setup)
{
  MIL << "============setup===========" << endl;
  TestSetup test(Arch_x86_64);
  // fake target from a subset of the online 11.1 repo
  test.loadTargetRepo(TESTS_SRC_DIR "/data/openSUSE-11.1_subset");
  test.loadRepo(TESTS_SRC_DIR "/data/openSUSE-11.1", "main");
  test.loadRepo(TESTS_SRC_DIR "/data/openSUSE-11.1_updates", "upd");
  test.loadRepo(TESTS_SRC_DIR "/data/misc", "misc");
  test.loadRepo(TESTS_SRC_DIR "/data/OBS_zypp_svn-11.1", "zypp");
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
// response: Already installed. Update to info-4.12-1.111. Update candidate
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
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NO_UPD_CANDIDATE));
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
