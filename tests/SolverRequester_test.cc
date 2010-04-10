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
  // fake target from the whole 11.1 repo
  test.loadTargetRepo(TESTS_SRC_DIR "/data/openSUSE-11.1_subset");
  test.loadRepo(TESTS_SRC_DIR "/data/openSUSE-11.1");
  test.loadRepo(TESTS_SRC_DIR "/data/openSUSE-11.1_updates");
}

///////////////////////////////////////////////////////////////////////////
// install
///////////////////////////////////////////////////////////////////////////

// request : install nonsense
// opts    : defaults
// response: not found by name, try caps, no cap found
BOOST_AUTO_TEST_CASE(install1)
{
  base::LogControl::TmpLineWriter shutUp(new log::FileLineWriter( "/tmp/zlog2"));
  MIL << "<=============install1==============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("nonsense");
  PackageArgs args(rawargs);
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME_TRYING_CAPS));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_CAP));
}

// request : install vim
// opts    : defaults
// response: vim set to install, no fallback to caps
BOOST_AUTO_TEST_CASE(install2)
{
  MIL << "<============install2===============>" << endl;

  vector<string> rawargs;
  rawargs.push_back("vim");
  PackageArgs args(rawargs);
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::SET_TO_INSTALL));
  BOOST_CHECK_EQUAL(sr.toInstall().size(), 1);
  BOOST_CHECK(hasPoolItem(sr.toInstall(), "vim", Edition("7.2-7.4.1"), Arch_x86_64));
}

///////////////////////////////////////////////////////////////////////////
// remove
///////////////////////////////////////////////////////////////////////////

// request : remove nonsense
// opts    : defaults
// response: not found by name, try caps, no cap found
BOOST_AUTO_TEST_CASE(remove1)
{
  MIL << "<===========================>" << endl;
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

// request : remove nonsense
// opts    : --name
// response: not found by name. Don't try caps.
BOOST_AUTO_TEST_CASE(remove2)
{
  MIL << "<===========================>" << endl;
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
// opts    : defaults
// response: not installed, fall back to caps, no provider installed
BOOST_AUTO_TEST_CASE(remove3)
{
  MIL << "<===========================>" << endl;
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
// opts    : defaults
// response: not installed, fall back to caps, no provider installed
// this one is done by sr.remove(vector<string>) instead of PackageArgs
BOOST_AUTO_TEST_CASE(remove4)
{
  MIL << "<===========================>" << endl;
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

// request : remove mc
// opts    : --name
// response: not installed. Don't fall back to caps.
BOOST_AUTO_TEST_CASE(remove5)
{
  MIL << "<===========================>" << endl;
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

/*
// request : remove libzypp
// opts    : defaults
// response: libzypp marked for removal, along with zypper, ...
BOOST_AUTO_TEST_CASE(remove5)
{
  MIL << "<===========================>" << endl;
  vector<string> rawargs;
  rawargs.push_back("libzypp");
  SolverRequester sr;

  sr.remove(rawargs);
  solve

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_INSTALLED));
}
*/


///////////////////////////////////////////////////////////////////////////
// update
///////////////////////////////////////////////////////////////////////////

// request : update vim
// opts    : defaults
// response: not installed
BOOST_AUTO_TEST_CASE(update1)
{
  MIL << "<===========================>" << endl;

  vector<string> rawargs;
  rawargs.push_back("vim");
  PackageArgs args(rawargs);
  SolverRequester sr;

  sr.update(rawargs);
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_INSTALLED));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME_TRYING_CAPS));
  BOOST_CHECK(!sr.hasFeedback(SolverRequester::Feedback::NO_INSTALLED_PROVIDER));
}


// request :
// opts    :
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
