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

BOOST_AUTO_TEST_CASE(setup)
{
  // enables logging for the scope of this block:
  base::LogControl::TmpLineWriter shutUp(new log::FileLineWriter( "/tmp/zlog"));

  TestSetup test(Arch_x86_64);
  // fake target from the whole 11.1 repo
  test.loadTargetRepo(TESTS_SRC_DIR "/data/openSUSE-11.1_subset");
  test.loadRepo(TESTS_SRC_DIR "/data/openSUSE-11.1");
  test.loadRepo(TESTS_SRC_DIR "/data/openSUSE-11.1_updates");
}


BOOST_AUTO_TEST_CASE(not_found)
{
  base::LogControl::TmpLineWriter shutUp(new log::FileLineWriter( "/tmp/zlog2"));

  vector<string> rawargs;
  rawargs.push_back("nonsense");
  PackageArgs args(rawargs);
  SolverRequester sr;

  sr.install(rawargs);

  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_NAME_TRYING_CAPS));
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_FOUND_CAP));
}


/*
BOOST_AUTO_TEST_CASE(simple_install)
{
  base::LogControl::TmpLineWriter shutUp(new log::FileLineWriter( "/tmp/zlog2"));

  vector<string> rawargs;
  rawargs.push_back("zypper");
  PackageArgs args(rawargs);
  SolverRequester sr;

  sr.install(rawargs);
}
*/

BOOST_AUTO_TEST_CASE(simple_update)
{
  base::LogControl::TmpLineWriter shutUp(new log::FileLineWriter( "/tmp/zlog2"));

  vector<string> rawargs;
  rawargs.push_back("vim");
  PackageArgs args(rawargs);
  SolverRequester sr;

  sr.update(rawargs);
  BOOST_CHECK(sr.hasFeedback(SolverRequester::Feedback::NOT_INSTALLED));
}

